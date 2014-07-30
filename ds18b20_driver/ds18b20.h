#ifndef __DS18B20_H
#define __DS18B20_H 
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

u8 DS18B20_Init(void);//初始化DS18B20
void DS18B20_Rst(void);//复位DS18B20   
short DS18B20_Get_Temp(void);//获取温度


#endif















