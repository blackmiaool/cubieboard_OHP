
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

#define RPUSBDISP_STATUS_BUFFER_SIZE 32
u32 io_addr;
struct rpusbdisp_dev {

    void * fb_handle;
    void * touch_handle;
    struct delayed_work completion_work;
};


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
completion_work.work);

    fbhandler_on_all_transfer_done(dev);
}


int rpusbdisp_usb_try_copy_area(struct rpusbdisp_dev * dev, int sx, int sy, int dx, int dy, int width, int height)
{
    printk("!!!!!rpusbdisp_usb_try_copy_area!!!!\n");
    return 1;

}

int rpusbdisp_usb_try_draw_rect(struct rpusbdisp_dev * dev, int x, int y, int right, int bottom, pixel_type_t color, int operation)
{
    printk("!!!!!rpusbdisp_usb_try_draw_rect!!!!\n");
    return 1;

}

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


static int rp_init()
{
    struct rpusbdisp_dev *dev = NULL;
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
    INIT_DELAYED_WORK(&dev->completion_work, _on_display_transfer_finished_delaywork);
    fbhandler_on_new_device(dev);
    touchhandler_on_new_device(dev);
    fbhandler_set_unsync_flag(dev);
    schedule_delayed_work(&dev->completion_work, 0);

    return 0;

    error:
        if (dev) {
            kfree(dev);
        }
    return 0;

}


int __init register_usb_handlers(void)
{
    rp_init();
    return 0;
}


void unregister_usb_handlers(void)
{
      iounmap((u8 *)io_addr);
}
