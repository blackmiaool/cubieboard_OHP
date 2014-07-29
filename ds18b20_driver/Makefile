ifneq ($(KERNELRELEASE),)
	ds18b20-objs := ds_core.o ds18b20op.o
	obj-m := ds18b20.o 
else
	KERNELDIR ?=/home/blackmiaool/pro/cb/kernel-source
	PWD := $(shell pwd)
default:
	$(MAKE) ARCH=arm CROSS_COMPILE=arm-none-linux-gnueabi- -C $(KERNELDIR) M=$(PWD) 
endif
clean:
	rm -f *.o
	rm -f *.mod.c
	rm -f *.ko
	rm -f .*.cmd
	rm -f *.c~
