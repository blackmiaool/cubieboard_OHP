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
#include "ds18b20.h"
#define GLOBALMEM_SIZE 0X1000
#define MEM_CLEAR 0X1
#define GLOBALMEM_MAJOR 0 
MODULE_LICENSE("Dual BSD/GPL");
extern char s;
uint16_t get_temp()
{
  s+=1;
}
struct ds18b20_dev
{
    struct cdev cdev;
    unsigned char value;
    void __iomem* gaddr;
    unsigned handler;
    unsigned int   ds18b20_handle;
};

int ds18b20_open(struct inode *inode,struct file *filp);
int ds18b20_release(struct inode *inode,struct file *filp);
ssize_t ds18b20_read(struct file *filp,char __user *buf, size_t count, \
                   loff_t* f_pos);
ssize_t ds18b20_write(struct file *filp,const char __user *buf, size_t count, \
                    loff_t* f_pos);
struct file_operations ds18b20_fops={
    .owner=THIS_MODULE,
    .read=ds18b20_read,
    .write=ds18b20_write,
    .open=ds18b20_open,
    .release=ds18b20_release
};
#define ioa(addr,a) (*(u32 *)((u8 *)addr+a))
static void setv(u32 base,u32 addr,u32 bit_cnt,u32 value,u32 offset)
{
    ioa(base,addr)&=(~(((1<<bit_cnt)-1)<<offset));
    ioa(base,addr)|=(value<<offset);
}
int ds18b20_open(struct inode *inode,struct file *filp)
{
    struct ds18b20_dev *dev;
    dev=container_of(inode->i_cdev,struct ds18b20_dev,cdev);
    filp->private_data=dev;
    DS18B20_Init();
  //  setv((u32)dev->gaddr,0x104,3,1,16);//PH20(LED)as OUTPUT
    setv((u32)dev->gaddr,0x6c,3,0,0);//PD0_SELECT as input

    return 0;
}
ssize_t ds18b20_write(struct file *filp,const char __user *buf, size_t count, \
                    loff_t* f_pos)
{
    struct ds18b20_dev *dev=filp->private_data;
    copy_from_user(&(dev->value),buf,1);
    if(dev->value<'2'&&dev->value>='0')
        setv((u32)dev->gaddr,0x10c,1,dev->value-0x30,20);//set led
    printk(KERN_NOTICE "value:%d",dev->value);
    printk(KERN_NOTICE "write");
    return 1;
}
int ds18b20_release(struct inode *inode,struct file *filp)
{
    return 0;
}
ssize_t ds18b20_read(struct file *filp,char __user *buf, size_t count, \
                   loff_t* f_pos)
{
    struct ds18b20_dev *dev=filp->private_data;
    u8 state[2];
  //  printk("cnt:%d,pos:%d\n",count,*f_pos);
    uint16_t temp=DS18B20_Get_Temp();
    printk("temp=%d\n",temp);
    if(*f_pos>=2)
    {
        return 0;
    }
    state[0]=temp/256;
    state[1]=temp%256;
    copy_to_user(buf,state,2);
    *f_pos+=2;

    return 2;
}



int ds18b20_major=0;
struct ds18b20_dev* ds18b20_devp;
static void ds18b20_setup_cdev(struct ds18b20_dev *dev,int index)
{
    int err,devno=MKDEV(ds18b20_major,index);
    cdev_init(&dev->cdev,&ds18b20_fops);
    dev->cdev.owner=THIS_MODULE;
    dev->cdev.ops=&ds18b20_fops;
    err=cdev_add(&dev->cdev,devno,1);
    if(err)
        printk(KERN_NOTICE"error add cdev");
}
int ds18b20_init(void)
{
    int result;
    dev_t dev;
    printk("hello_fuck\r\n");
    dev=MKDEV(ds18b20_major,0);
    result=alloc_chrdev_region(&dev,0,1,"ds18b20");
    ds18b20_major=MAJOR(dev);
    ds18b20_devp=kmalloc(sizeof(struct ds18b20_dev),GFP_KERNEL);
    if(!ds18b20_devp)
    {
        result=-ENOMEM;
        goto fail_malloc;
    }
    memset(ds18b20_devp,0,sizeof(struct ds18b20_dev));
    ds18b20_setup_cdev(ds18b20_devp,0);
    ds18b20_devp->gaddr=ioremap(0x01c20800,0x400);
    setv((u32)ds18b20_devp->gaddr,0x6c,3,1,4);//PD21 as output
    setv((u32)ds18b20_devp->gaddr,0x7c,1,1,1);//set PD21
    return 0;
fail_malloc:
    unregister_chrdev_region(dev,ds18b20_devp);
    return -1;

}
void ds18b20_exit(void)
{

    cdev_del(&ds18b20_devp->cdev);
    iounmap(ds18b20_devp->gaddr);
    kfree(ds18b20_devp);
    unregister_chrdev_region(MKDEV(ds18b20_major,0),1);

}

module_init(ds18b20_init);
module_exit(ds18b20_exit);

