#include "ds18b20.h"
#include <linux/delay.h>

#define ioa(addr,a) (*(u32 *)((u8 *)addr+a))
void setv(u32 base,u32 addr,u32 bit_cnt,u32 value,u32 offset)
{
    ioa(base,addr)&=(~(((1<<bit_cnt)-1)<<offset));
    ioa(base,addr)|=(value<<offset);
}
struct ds18b20_dev
{
    struct cdev cdev;
    unsigned char value;
    void __iomem* gaddr;
    unsigned handler;
    unsigned int   ds18b20_handle;
};
extern struct ds18b20_dev* ds18b20_devp;
#define DS18B20_IO_IN()  (setv((u32)ds18b20_devp->gaddr,0x6c,3,0,0))//PD0_SELECT as input
#define DS18B20_IO_OUT() (setv((u32)ds18b20_devp->gaddr,0x6c,3,1,0))//PD0_SELECT as output
#define DS18B20_DQ_OUT_H  (setv((u32)ds18b20_devp->gaddr,0x7c,1,1,0))
#define DS18B20_DQ_OUT_L  (setv((u32)ds18b20_devp->gaddr,0x7c,1,0,0))
#define DS18B20_DQ_IN  (ioa(ds18b20_devp->gaddr,0x7c)&(1))

void DS18B20_Start(void);//¿ªÊ¼ÎÂ¶È×ª»»
void DS18B20_Write_Byte(u8 dat);//Ð´ÈëÒ»¸ö×Ö½Ú
u8 DS18B20_Read_Byte(void);//¶Á³öÒ»¸ö×Ö½Ú
u8 DS18B20_Read_Bit(void);//¶Á³öÒ»¸öÎ»
u8 DS18B20_Check(void);//¼ì²âÊÇ·ñ´æÔÚDS18B20 
u8 s;

void DS18B20_Rst(void)	   
{                 
	DS18B20_IO_OUT(); //SET PA0 OUTPUT
    DS18B20_DQ_OUT_L; //À­µÍDQ
    udelay(750);    //À­µÍ750us
    DS18B20_DQ_OUT_H; //DQ=1 
	udelay(15);     //15US
}

u8 DS18B20_Check(void) 	   
{   
	u8 retry=0;
	DS18B20_IO_IN();//SET PA0 INPUT	 
    while (DS18B20_DQ_IN&&retry<200)
	{
		retry++;
		udelay(1);
	};	 
	if(retry>=200)return 1;
	else retry=0;
    while (!DS18B20_DQ_IN&&retry<240)
	{
		retry++;
		udelay(1);
	};
	if(retry>=240)return 1;	    
	return 0;
}

u8 DS18B20_Read_Bit(void) 			 // read one bit
{
    u8 data;
	DS18B20_IO_OUT();//SET PA0 OUTPUT
    DS18B20_DQ_OUT_L; 
	udelay(2);
    DS18B20_DQ_OUT_H; 
	DS18B20_IO_IN();//SET PA0 INPUT
	udelay(12);
	if(DS18B20_DQ_IN)data=1;
    else data=0;	 
    udelay(50);           
    return data;
}

u8 DS18B20_Read_Byte(void)    // read one byte
{        
    u8 i,j,dat;
    dat=0;
	for (i=1;i<=8;i++) 
	{
        j=DS18B20_Read_Bit();
        dat=(j<<7)|(dat>>1);
    }						    
    return dat;
}

void DS18B20_Write_Byte(u8 dat)     
 {             
    u8 j;
    u8 testb;
	DS18B20_IO_OUT();//SET PA0 OUTPUT;
    for (j=1;j<=8;j++) 
	{
        DS18B20_DQ_OUT_L;// Write 1
            udelay(1);  
        testb=dat&0x01;
        dat=dat>>1;
        if (testb) 
        {                         
            DS18B20_DQ_OUT_H;          
        }
        udelay(60);
        DS18B20_DQ_OUT_H;
        udelay(15);
    }
}

void DS18B20_Start(void)// ds1820 start convert
{   						               
    DS18B20_Rst();	   
	DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0x44);// convert
} 
   	 
u8 DS18B20_Init(void)
{
	DS18B20_Rst();
	return DS18B20_Check();
}  

short DS18B20_Get_Temp(void)
{
    u8 temp;
    u8 TL,TH;
	static short tem;
repeat:
    DS18B20_Start ();                    // ds1820 start convert
    DS18B20_Rst();
    DS18B20_Check();	 
    DS18B20_Write_Byte(0xcc);// skip rom
    DS18B20_Write_Byte(0xbe);// convert	    
    TL=DS18B20_Read_Byte(); // LSB   
    TH=DS18B20_Read_Byte(); // MSB  
	    	  
    if(TH>7)
    {
        TH=~TH;
        TL=~TL; 
        temp=0;//ÎÂ¶ÈÎª¸º  
    }else temp=1;//ÎÂ¶ÈÎªÕý	  	  
    tem=TH; //»ñµÃ¸ß°ËÎ»
    tem<<=8;    
    tem+=TL;//»ñµÃµ×°ËÎ»
    tem=tem*5/8;//×ª»»     
    if(tem>5&&tem<450&&temp)
    {
        if(temp)
            return tem; //·µ»ØÎÂ¶ÈÖµ
        else 
            return -tem; 
    }
    else
        goto repeat;
	   
} 
 
