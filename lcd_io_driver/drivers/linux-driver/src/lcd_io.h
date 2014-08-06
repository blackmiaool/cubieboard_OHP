#ifndef __LCD_H
#define __LCD_H	
#include "inc/common.h"	 

#define R0             0x00
#define R1             0x01
#define R2             0x02
#define R3             0x03
#define R4             0x04
#define R5             0x05
#define R6             0x06
#define R7             0x07
#define R8             0x08
#define R9             0x09
#define R10            0x0A
#define R12            0x0C
#define R13            0x0D
#define R14            0x0E
#define R15            0x0F
#define R16            0x10
#define R17            0x11
#define R18            0x12
#define R19            0x13
#define R20            0x14
#define R21            0x15
#define R22            0x16
#define R23            0x17
#define R24            0x18
#define R25            0x19
#define R26            0x1A
#define R27            0x1B
#define R28            0x1C
#define R29            0x1D
#define R30            0x1E
#define R31            0x1F
#define R32            0x20
#define R33            0x21
#define R34            0x22
#define R36            0x24
#define R37            0x25
#define R40            0x28
#define R41            0x29
#define R43            0x2B
#define R45            0x2D
#define R48            0x30
#define R49            0x31
#define R50            0x32
#define R51            0x33
#define R52            0x34
#define R53            0x35
#define R54            0x36
#define R55            0x37
#define R56            0x38
#define R57            0x39
#define R59            0x3B
#define R60            0x3C
#define R61            0x3D
#define R62            0x3E
#define R63            0x3F
#define R64            0x40
#define R65            0x41
#define R66            0x42
#define R67            0x43
#define R68            0x44
#define R69            0x45
#define R70            0x46
#define R71            0x47
#define R72            0x48
#define R73            0x49
#define R74            0x4A
#define R75            0x4B
#define R76            0x4C
#define R77            0x4D
#define R78            0x4E
#define R79            0x4F
#define R80            0x50
#define R81            0x51
#define R82            0x52
#define R83            0x53
#define R96            0x60
#define R97            0x61
#define R106           0x6A
#define R118           0x76
#define R128           0x80
#define R129           0x81
#define R130           0x82
#define R131           0x83
#define R132           0x84
#define R133           0x85
#define R134           0x86
#define R135           0x87
#define R136           0x88
#define R137           0x89
#define R139           0x8B
#define R140           0x8C
#define R141           0x8D
#define R143           0x8F
#define R144           0x90
#define R145           0x91
#define R146           0x92
#define R147           0x93
#define R148           0x94
#define R149           0x95
#define R150           0x96
#define R151           0x97
#define R152           0x98
#define R153           0x99
#define R154           0x9A
#define R157           0x9D
#define R192           0xC0
#define R193           0xC1
#define R229           0xE5		
void lcd_init(void);
void LCD_WriteRAM_Prepare(void);
extern u32 io_addr;
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 *color);
#define ioa(addr,a) (*(u32 *)((u8 *)addr+a))
__inline void setv(u32 base,u32 addr,u32 bit_cnt,u32 value,u32 offset);
#define getv(base,addr,mask) (ioa(base,addr)&mask)
#define io_set(base,addr,mask) (ioa(base,addr)|=mask)
#define io_clear(base,addr,mask) (ioa(base,addr)&=(~mask))
////////////////////////////////////////////////////////////////////
//-----------------LCD端口定义---------------- 
#define	LCD_LED //PCout(10) //LCD背光    		 PC10 
//RS 17
//CS 18
//WR 20
//RST 19
//RD 23
#define LCD_RST_H io_set  (io_addr,0x7c,0x80000L)	
#define LCD_RST_L io_clear(io_addr,0x7c,0x80000L)	
#define	LCD_CS_H  io_set  (io_addr,0x7c,0x40000L)	
#define	LCD_CS_L  io_clear(io_addr,0x7c,0x40000L)		
#define	LCD_RS_H  io_set  (io_addr,0x7c,0x20000L)		  
#define	LCD_RS_L  io_clear(io_addr,0x7c,0x20000L)		  
#define	LCD_WR_H  io_set  (io_addr,0x7c,0x100000L)		
#define	LCD_WR_L  io_clear(io_addr,0x7c,0x100000L)	
#define	LCD_RD_H  io_set  (io_addr,0x7c,0x800000L)		
#define	LCD_RD_L  io_clear(io_addr,0x7c,0x800000L)		
								    
//PB0~15,作为数据线
#define DATAOUT(x) setv(io_addr,0x7c,16,x,0)//数据输出
#define DATAIN     getv(io_addr,0x7c,0xffff)  //数据输入

#define USE_HORIZONTAL 1
#define LCD_WR_DATA(data){\
LCD_RS_H;\
LCD_CS_L;\
DATAOUT(data);\
LCD_WR_L;\
LCD_WR_H;\
LCD_CS_H;\
} 
__inline void LCD_DrawPoint(u16 x,u16 y,u16 color);
__inline void LCD_SetCursor(u16 Xpos, u16 Ypos);
__inline void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue);
__inline void LCD_WR_REG(u8 data);											 
//9320/9325 LCD寄存器  
					  		 
#endif  
	 
	 



