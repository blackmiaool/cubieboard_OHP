#include <linux/module.h>
#include <linux/types.h>
#include <linux/fs.h>
#include <linux/mm.h>
#include <linux/sched.h>
#include <linux/init.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/sched.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <asm/io.h>
#include <asm/system.h>
#include <asm/uaccess.h>
#include <linux/platform_device.h>
#include <linux/gpio.h>
#include <mach/hardware.h>
#include <mach/system.h>
#include <mach/irqs.h>
#include "plat/sys_config.h"
#include "/home/blackmiaool/pro/cb/kernel-source/drivers/gpio/gpio-sunxi.h"
#define GLOBALMEM_SIZE 0X1000
#define MEM_CLEAR 0X1
#define GLOBALMEM_MAJOR 0 
MODULE_LICENSE("Dual BSD/GPL");
extern char s;
uint16_t get_temp()
{
  s+=1;
}
struct scull_dev
{
    struct cdev cdev;
    unsigned char value;
    void __iomem* gaddr;
    unsigned handler;
    unsigned int   scull_handle;
};

int scull_open(struct inode *inode,struct file *filp);
int scull_release(struct inode *inode,struct file *filp);
ssize_t scull_read(struct file *filp,char __user *buf, size_t count, \
                   loff_t* f_pos);
ssize_t scull_write(struct file *filp,const char __user *buf, size_t count, \
                    loff_t* f_pos);
struct file_operations scull_fops={
    .owner=THIS_MODULE,
    .read=scull_read,
    .write=scull_write,
    .open=scull_open,
    .release=scull_release
};
#define ioa(addr,a) (*(u32 *)((u8 *)addr+a))
void setv(u32 base,u32 addr,u32 bit_cnt,u32 value,u32 offset)
{
    ioa(base,addr)&=(~(((1<<bit_cnt)-1)<<offset));
    ioa(base,addr)|=(value<<offset);
}
int scull_open(struct inode *inode,struct file *filp)
{
    struct scull_dev *dev;
    dev=container_of(inode->i_cdev,struct scull_dev,cdev);
    filp->private_data=dev;
    setv((u32)dev->gaddr,0x104,3,1,16);//PH20(LED)as OUTPUT
    setv((u32)dev->gaddr,0x6c,3,0,0);//PD0_SELECT as input

    return 0;
}
ssize_t scull_write(struct file *filp,const char __user *buf, size_t count, \
                    loff_t* f_pos)
{
    struct scull_dev *dev=filp->private_data;
    copy_from_user(&(dev->value),buf,1);
    if(dev->value<'2'&&dev->value>='0')
        setv((u32)dev->gaddr,0x10c,1,dev->value-0x30,20);//set led
    printk(KERN_NOTICE "value:%d",dev->value);
    printk(KERN_NOTICE "write");
    return 1;
}
int scull_release(struct inode *inode,struct file *filp)
{
    return 0;
}
ssize_t scull_read(struct file *filp,char __user *buf, size_t count, \
                   loff_t* f_pos)
{
    struct scull_dev *dev=filp->private_data;
    u8 state[2];
    printk("cnt:%d,pos:%d\n",count,*f_pos);
    if(*f_pos>=2)
    {
        return 0;
    }
    state[0]=(ioa(dev->gaddr,0x7c)&(1))+0x30;
    state[1]=0;
    copy_to_user(buf,state,1);
//    if(*f_pos>=1)
//        return count ? -ENXIO:0;
//    if(count>1-*f_pos)
//        count=1-*f_pos;
    *f_pos+=2;

    return 2;
}



int scull_major=0;
struct scull_dev* scull_devp;
static void scull_setup_cdev(struct scull_dev *dev,int index)
{
    int err,devno=MKDEV(scull_major,index);
    cdev_init(&dev->cdev,&scull_fops);
    dev->cdev.owner=THIS_MODULE;
    dev->cdev.ops=&scull_fops;
    err=cdev_add(&dev->cdev,devno,1);
    if(err)
        printk(KERN_NOTICE"error add cdev");
}
int globalmem_init(void)
{
    int result;
    dev_t dev;
    printk("hello_fuck\r\n");
    dev=MKDEV(scull_major,0);
    result=alloc_chrdev_region(&dev,0,1,"scull");
    scull_major=MAJOR(dev);
    scull_devp=kmalloc(sizeof(struct scull_dev),GFP_KERNEL);
    if(!scull_devp)
    {
        result=-ENOMEM;
        goto fail_malloc;
    }
    memset(scull_devp,0,sizeof(struct scull_dev));
    scull_setup_cdev(scull_devp,0);
    scull_devp->gaddr=ioremap(0x01c20800,0x400);
    setv((u32)scull_devp->gaddr,0x6c,3,1,4);//PD21 as output
    setv((u32)scull_devp->gaddr,0x7c,1,1,1);//set PD21
    return 0;
fail_malloc:
    unregister_chrdev_region(dev,scull_devp);
    return -1;

}
void globalmem_exit(void)
{

    cdev_del(&scull_devp->cdev);
    iounmap(scull_devp->gaddr);
    kfree(scull_devp);
    unregister_chrdev_region(MKDEV(scull_major,0),1);

}

module_init(globalmem_init);
module_exit(globalmem_exit);

