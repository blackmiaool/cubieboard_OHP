
/*
* RoboPeak USB LCD Display Linux Driver
*
* Copyright (C) 2009 - 2013 RoboPeak Team
* This file is licensed under the GPL. See LICENSE in the package.
*
* http://www.robopeak.net
*
* Author Shikai Chen
* -----------------------------------------------------------------
* USB Driver Implementations
*/

#include "inc/common.h"
#include "inc/usbhandlers.h"
#include "inc/fbhandlers.h"
#include "inc/touchhandlers.h"
#include "lcd_io.h"
#define DL_ALIGN_UP(x, a) ALIGN(x, a)
#define DL_ALIGN_DOWN(x, a) ALIGN(x-(a-1), a)


static const struct usb_device_id id_table[] = {
{
          .idVendor = RP_DISP_USB_VENDOR_ID,
          .idProduct = RP_DISP_USB_PRODUCT_ID,
          .match_flags = USB_DEVICE_ID_MATCH_VENDOR | USB_DEVICE_ID_MATCH_PRODUCT,
        },
{ },
};

static atomic_t _devlist_count = ATOMIC_INIT(0);
static LIST_HEAD( rpusbdisp_list );
static DEFINE_MUTEX(_mutex_usbdevlist);
static DECLARE_WAIT_QUEUE_HEAD(_usblist_waitqueue);

#if 0
static struct task_struct * _usb_status_polling_task;
static int _kthread_usb_status_poller_proc(void *data);
#endif

static volatile int _working_flag = 0;


#define RPUSBDISP_STATUS_BUFFER_SIZE 32


struct rpusbdisp_disp_ticket_bundle {
    int ticket_count;
    struct list_head ticket_list;

};

struct rpusbdisp_disp_ticket {
    struct urb * transfer_urb;
    struct list_head ticket_list_node;
    struct rpusbdisp_dev * binded_dev;
    
};


struct rpusbdisp_disp_ticket_pool {
    struct list_head list;
    spinlock_t oplock;
    size_t disp_urb_count;
    size_t packet_size_factor;
    int availiable_count;
    wait_queue_head_t wait_queue;
    struct delayed_work completion_work;

};

struct rpusbdisp_dev {
    // timing and sync
    struct list_head dev_list_node;
    int dev_id;
    struct mutex op_locker;


    // usb device info
    struct usb_device * udev;
    struct usb_interface * interface;
    

    // status package related
    __u8 status_in_buffer[RPUSBDISP_STATUS_BUFFER_SIZE]; // data buffer for the status IN endpoint
    size_t status_in_buffer_recvsize;

        

    __u8 status_in_ep_addr;
    wait_queue_head_t status_wait_queue;
    struct urb * urb_status_query;
    int urb_status_fail_count;


    // display data related
    __u8 disp_out_ep_addr;
    size_t disp_out_ep_max_size;

    struct rpusbdisp_disp_ticket_pool disp_tickets_pool;



    void * fb_handle;
    void * touch_handle;

    __u16 device_fwver;
};

#if 0

/*
* usb class driver info in order to get a minor number from the usb core,
* and to have the device registered with the driver core
*/
static struct usb_class_driver lcd_class = {
.name = "rp_usbdisp%d",
.fops = &disp_fops,
.minor_base = RPUSBDISP_MINOR,
};

#endif


static int _return_disp_tickets(struct rpusbdisp_dev * dev, struct rpusbdisp_disp_ticket * ticket);


struct device * rpusbdisp_usb_get_devicehandle(struct rpusbdisp_dev *dev)
{
    return &dev->udev->dev;
}

void rpusbdisp_usb_set_fbhandle(struct rpusbdisp_dev * dev, void * fbhandle)
{
    dev->fb_handle = fbhandle;
}
 
void * rpusbdisp_usb_get_fbhandle(struct rpusbdisp_dev * dev)
{
    return dev->fb_handle;
}

void rpusbdisp_usb_set_touchhandle(struct rpusbdisp_dev * dev, void * touch_handle)
{
    dev->touch_handle = touch_handle;
}

void * rpusbdisp_usb_get_touchhandle(struct rpusbdisp_dev * dev)
{
    return dev->touch_handle;
}




static void _on_display_transfer_finished_delaywork(struct work_struct *work)
{
    struct rpusbdisp_dev * dev = container_of(work, struct rpusbdisp_dev,
disp_tickets_pool.completion_work.work);

    fbhandler_on_all_transfer_done(dev);
}

static void _on_display_transfer_finished(struct urb *urb)
{
    struct rpusbdisp_disp_ticket * ticket = (struct rpusbdisp_disp_ticket *)urb->context;
    struct rpusbdisp_dev * dev = ticket->binded_dev;
    schedule_delayed_work(&dev->disp_tickets_pool.completion_work, 0);
}



static int _sell_disp_tickets(struct rpusbdisp_dev * dev, struct rpusbdisp_disp_ticket_bundle * bundle, size_t required_count)
{
    unsigned long irq_flags;
    int ans = 0;
    struct list_head *node;
    // do not sell tickets when the device has been closed

    if (required_count == 0) {
        printk("required for zero ?!\n");
        return 0;
    }
    spin_lock_irqsave(&dev->disp_tickets_pool.oplock, irq_flags);
    do {
        if (required_count > dev->disp_tickets_pool.availiable_count) {
            // no enough tickets availiable
            break;
        }

        dev->disp_tickets_pool.availiable_count -= required_count;
        
        bundle->ticket_count = required_count;
        ans = bundle->ticket_count;
        
        INIT_LIST_HEAD(&bundle->ticket_list);
        while(required_count--) {
            node = dev->disp_tickets_pool.list.next;
            list_del_init(node);
            list_add_tail(node, &bundle->ticket_list);
        }
    }while(0);
    spin_unlock_irqrestore(&dev->disp_tickets_pool.oplock,irq_flags);
    return ans;
}


static int _return_disp_tickets(struct rpusbdisp_dev * dev, struct rpusbdisp_disp_ticket * ticket)
{
    int all_finished = 0;
    unsigned long irq_flags;
    // insert to the available queue
    
    spin_lock_irqsave(&dev->disp_tickets_pool.oplock, irq_flags);
list_add_tail(&ticket->ticket_list_node, &dev->disp_tickets_pool.list);

    if ( ++dev->disp_tickets_pool.availiable_count == dev->disp_tickets_pool.disp_urb_count) {
        all_finished = 1;
    }

spin_unlock_irqrestore(&dev->disp_tickets_pool.oplock, irq_flags);


    wake_up(&dev->disp_tickets_pool.wait_queue);
    return all_finished;
}


int rpusbdisp_usb_try_copy_area(struct rpusbdisp_dev * dev, int sx, int sy, int dx, int dy, int width, int height)
{
    
    struct rpusbdisp_disp_ticket_bundle bundle;
    struct rpusbdisp_disp_ticket * ticket;
    rpusbdisp_disp_copyarea_packet_t * cmd_copyarea;
    // only one ticket is enough
    if (!_sell_disp_tickets(dev, &bundle, 1)) {
        // tickets is inadequate, try next time
        return 0;
    }

    BUG_ON(sizeof(rpusbdisp_disp_copyarea_packet_t) > dev->disp_out_ep_max_size);

    ticket = list_entry(bundle.ticket_list.next, struct rpusbdisp_disp_ticket, ticket_list_node);
    
    cmd_copyarea = (rpusbdisp_disp_copyarea_packet_t *)ticket->transfer_urb->transfer_buffer;
    cmd_copyarea->header.cmd_flag = RPUSBDISP_DISPCMD_COPY_AREA| RPUSBDISP_CMD_FLAG_START;

    cmd_copyarea->sx = cpu_to_le16(sx);
    cmd_copyarea->sy = cpu_to_le16(sy);
    cmd_copyarea->dx = cpu_to_le16(dx);
    cmd_copyarea->dy = cpu_to_le16(dy);

    cmd_copyarea->width= cpu_to_le16(width);
    cmd_copyarea->height = cpu_to_le16(height);

    //add one more byte to bypass usbdisp 1.03 fw bug
    ticket->transfer_urb->transfer_buffer_length = sizeof(rpusbdisp_disp_copyarea_packet_t) + 1;

    if (usb_submit_urb(ticket->transfer_urb, GFP_KERNEL)) {
        // submit failure,
         _on_display_transfer_finished(ticket->transfer_urb);
        return 0;
    }
    
    return 1;

}

int rpusbdisp_usb_try_draw_rect(struct rpusbdisp_dev * dev, int x, int y, int right, int bottom, pixel_type_t color, int operation)
{
    rpusbdisp_disp_fillrect_packet_t * cmd_fillrect;
    struct rpusbdisp_disp_ticket_bundle bundle;
    struct rpusbdisp_disp_ticket * ticket;

    // only one ticket is enough
    if (!_sell_disp_tickets(dev, &bundle, 1)) {
        // tickets is inadequate, try next time
        return 0;
    }

    BUG_ON(sizeof(rpusbdisp_disp_fillrect_packet_t) > dev->disp_out_ep_max_size);

    ticket = list_entry(bundle.ticket_list.next, struct rpusbdisp_disp_ticket, ticket_list_node);
    
    cmd_fillrect = (rpusbdisp_disp_fillrect_packet_t *)ticket->transfer_urb->transfer_buffer;

    cmd_fillrect->header.cmd_flag = RPUSBDISP_DISPCMD_RECT | RPUSBDISP_CMD_FLAG_START;

    cmd_fillrect->left = cpu_to_le16(x);
    cmd_fillrect->top = cpu_to_le16(y);
    cmd_fillrect->right = cpu_to_le16(right);
    cmd_fillrect->bottom = cpu_to_le16(bottom);

    cmd_fillrect->color_565 = cpu_to_le16(color);
    cmd_fillrect->operation = operation;

    ticket->transfer_urb->transfer_buffer_length = sizeof(rpusbdisp_disp_fillrect_packet_t);

    if (usb_submit_urb(ticket->transfer_urb, GFP_KERNEL)) {
        // submit failure,
         _on_display_transfer_finished(ticket->transfer_urb);
        return 0;
    }
    
    return 1;

}





// context used by the bitblt display packet encoder
struct bitblt_encoding_context_t {
    struct rpusbdisp_disp_ticket_bundle bundle;
    struct rpusbdisp_disp_ticket * ticket;
    struct list_head * current_node;

    size_t encoded_pos;
    size_t packet_pos;
    _u8 * urbbuffer ;
    int rlemode;

} ;


struct rle_encoder_context {
    struct bitblt_encoding_context_t * encoder_ctx;
    int is_common_section;
    size_t section_size;
    pixel_type_t section_data[128];

};


u32 io_addr;
int rpusbdisp_usb_try_send_image(struct rpusbdisp_dev * dev, const pixel_type_t * framebuffer, int x, int y, int right, int bottom, int line_width, int clear_dirty)
{

    int last_copied_x, last_copied_y;

    // estimate how many tickets are needed
    const size_t image_size = (right-x + 1)* (bottom-y+1) * (RP_DISP_DEFAULT_PIXEL_BITS/8);
        printk("imaage\n");
    printk("1sx=%d",x);
    printk("y=%d",y);
    printk("right=%d",right);
    printk("bottom=%d",bottom);
    printk("line_width=%d",line_width);
    // do not transmit zero size image
    if (!image_size) return 1;
    framebuffer += (y*line_width + x);
    {
        for (last_copied_x = right; last_copied_x >= x; --last_copied_x) {
            const pixel_type_t *buf=framebuffer;
            LCD_SetCursor(last_copied_x,y);
            LCD_WriteRAM_Prepare();
            for (last_copied_y = y; last_copied_y <= bottom; ++last_copied_y) {
                LCD_WR_DATA(*buf);  
               buf+=320;  
            }
            framebuffer += 1;
        }  
    }
    return 1;
}

static void _on_release_disp_tickets_pool(struct rpusbdisp_dev * dev)
{
    unsigned long irq_flags;
    struct rpusbdisp_disp_ticket * ticket;
    struct list_head *node;
    int tickets_count = dev->disp_tickets_pool.disp_urb_count;
    
    dev_info(&dev->interface->dev, "waiting for all tickets to be finished...\n");

    

    while(tickets_count) {
        
        spin_lock_irqsave(&dev->disp_tickets_pool.oplock, irq_flags);
        if (dev->disp_tickets_pool.availiable_count) {
            --dev->disp_tickets_pool.availiable_count;
        } else {
            spin_unlock_irqrestore(&dev->disp_tickets_pool.oplock,irq_flags);
            sleep_on_timeout(&dev->disp_tickets_pool.wait_queue, 2*HZ);
            continue;
        }
        node = dev->disp_tickets_pool.list.next;
        list_del_init(node);
        spin_unlock_irqrestore(&dev->disp_tickets_pool.oplock,irq_flags);

        ticket = list_entry(node, struct rpusbdisp_disp_ticket, ticket_list_node);
      
usb_free_coherent(ticket->transfer_urb->dev, RPUSBDISP_MAX_TRANSFER_SIZE,
ticket->transfer_urb->transfer_buffer, ticket->transfer_urb->transfer_dma);

        usb_free_urb(ticket->transfer_urb);
        kfree(ticket);
        --tickets_count;
    }

}

static int _on_alloc_disp_tickets_pool(struct rpusbdisp_dev * dev)
{
    struct rpusbdisp_disp_ticket * newborn;
    int actual_allocated = 0;
    _u8 * transfer_buffer;
    size_t packet_size_factor;
    size_t ticket_logic_size;

    packet_size_factor = (RPUSBDISP_MAX_TRANSFER_SIZE/dev->disp_out_ep_max_size) ;
    ticket_logic_size = packet_size_factor * dev->disp_out_ep_max_size ;

    
    spin_lock_init(&dev->disp_tickets_pool.oplock);
    INIT_LIST_HEAD(&dev->disp_tickets_pool.list);

    while(actual_allocated < RPUSBDISP_MAX_TRANSFER_TICKETS_COUNT) {
        newborn = kzalloc(sizeof(struct rpusbdisp_disp_ticket), GFP_KERNEL);
        if (!newborn) {
            break;
        }
        
        newborn->transfer_urb = usb_alloc_urb(0, GFP_KERNEL);
        if (!newborn->transfer_urb) {
            kfree(newborn);
            break;
        }
        
      

        transfer_buffer = usb_alloc_coherent(dev->udev, RPUSBDISP_MAX_TRANSFER_SIZE, GFP_KERNEL, &newborn->transfer_urb->transfer_dma);
        if (!transfer_buffer) {
            usb_free_urb(newborn->transfer_urb);
            kfree(newborn);
            break;
        }

        
        // setup urb
        usb_fill_bulk_urb(newborn->transfer_urb, dev->udev, usb_sndbulkpipe(dev->udev, dev->disp_out_ep_addr),
            transfer_buffer, ticket_logic_size, _on_display_transfer_finished, newborn);
        newborn->transfer_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

        
        newborn->binded_dev = dev;

        dev_info(&dev->interface->dev, "allocated ticket %p with urb %p\n", newborn, newborn->transfer_urb);


        list_add_tail(&newborn->ticket_list_node, &dev->disp_tickets_pool.list);

        ++actual_allocated;
        
    }


    init_waitqueue_head(&dev->disp_tickets_pool.wait_queue);
    dev->disp_tickets_pool.disp_urb_count = actual_allocated;
    dev->disp_tickets_pool.availiable_count = actual_allocated;
    dev->disp_tickets_pool.packet_size_factor = packet_size_factor;
   // dev_info(&dev->interface->dev, "allocated %d urb tickets for transfering display data. %lu size each\n", actual_allocated, ticket_logic_size);
    printk("allocated\n");
    return actual_allocated?0:-ENOMEM;
};

static int _on_new_usb_device(struct rpusbdisp_dev * dev)
{



    return 0;
}


static void _on_del_usb_device(struct rpusbdisp_dev * dev)
{


    //_del_usbdev_from_list(de  
    _on_release_disp_tickets_pool(dev);
   
    usb_free_urb(dev->urb_status_query);
    dev->urb_status_query = NULL;
     
    dev_info(&dev->interface->dev, "RP USB Display (#%d) now disconnected\n", dev->dev_id);
}


int rpusbdisp_usb_get_device_count(void)
{
    return atomic_read(&_devlist_count);
}

static int rp_init()
{
struct rpusbdisp_dev *dev = NULL;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    size_t buffer_size;
    int i;
    int retval = -ENOMEM;

    /* allocate memory for our device state and initialize it */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (dev == NULL) {
    err("Out of memory");
    goto error;
    }
    io_addr=ioremap((u8 *)0x01c20800,0x400);
    lcd_init();
    // dev->umake
 
    // add the device to the list
    INIT_DELAYED_WORK(&dev->disp_tickets_pool.completion_work, _on_display_transfer_finished_delaywork);
    fbhandler_on_new_device(dev);
    touchhandler_on_new_device(dev);
    fbhandler_set_unsync_flag(dev);
    schedule_delayed_work(&dev->disp_tickets_pool.completion_work, 0);

    return 0;

    error:
        if (dev) {
            kfree(dev);
        }
    return retval;

}
static int rpusbdisp_probe(struct usb_interface *interface, const struct usb_device_id *id)
{

    struct rpusbdisp_dev *dev = NULL;
    struct usb_host_interface *iface_desc;
    struct usb_endpoint_descriptor *endpoint;
    size_t buffer_size;
    int i;
    int retval = -ENOMEM;

    /* allocate memory for our device state and initialize it */
    dev = kzalloc(sizeof(*dev), GFP_KERNEL);
    if (dev == NULL) {
    err("Out of memory");
    goto error;
    }
    io_addr=ioremap((u8 *)0x01c20800,0x400);
    lcd_init();
    // dev->umake
    dev = usb_get_dev(interface_to_usbdev(interface));
    // dev->interface = interface;
    usb_set_intfdata(interface, dev);

    // add the device to the list
    INIT_DELAYED_WORK(&dev->disp_tickets_pool.completion_work, _on_display_transfer_finished_delaywork);
    fbhandler_on_new_device(dev);
    touchhandler_on_new_device(dev);
    fbhandler_set_unsync_flag(dev);
    schedule_delayed_work(&dev->disp_tickets_pool.completion_work, 0);

    return 0;

    error:
        if (dev) {
            kfree(dev);
        }
    return retval;
}



static int rpusbdisp_suspend(struct usb_interface *intf, pm_message_t message)
{
struct usb_lcd *dev = usb_get_intfdata(intf);

if (!dev)
return 0;
    // not implemented yet
return 0;
}


static int rpusbdisp_resume (struct usb_interface *intf)
{
    // not implemented yet
return 0;
}

static void rpusbdisp_disconnect(struct usb_interface *interface)
{
struct rpusbdisp_dev *dev;
  iounmap((u8 *)io_addr);
dev = usb_get_intfdata(interface);
usb_set_intfdata(interface, NULL);
    _on_del_usb_device(dev);

kfree(dev);
}


static struct usb_driver usbdisp_driver = {
.name = RP_DISP_DRIVER_NAME,
.probe = rpusbdisp_probe,
.disconnect = rpusbdisp_disconnect,
.suspend = rpusbdisp_suspend,
.resume = rpusbdisp_resume,
.id_table = id_table,
.supports_autosuspend = 0,
};

int __init register_usb_handlers(void)
{
    // create the status polling task

    _working_flag = 1;
#if 0
_usb_status_polling_task = kthread_run(_kthread_usb_status_poller_proc, NULL, "rpusbdisp_worker%d", 0);
if (IS_ERR(_usb_status_polling_task)) {
err("Cannot create the kernel worker thread!\n");
return -ENOMEM;
}
#endif
rp_init();
    return 0;//usb_register(&usbdisp_driver);
}


void unregister_usb_handlers(void)
{

    // cancel the worker thread
    _working_flag = 0;


    usb_deregister(&usbdisp_driver);
}
