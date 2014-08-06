#include "lcd_io.h"

//¶¨ÒåLCDµÄ³ß´ç
#define LCD_W 320
#define LCD_H 240



//////////////////////////////////////////////////////////////////////
//»­±ÊÑÕÉ«
#define WHITE         	 0xFFFF
#define BLACK         	 0x0000	  
#define BLUE         	 0x001F  
#define BRED             0XF81F
#define GRED 			 0XFFE0
#define GBLUE			 0X07FF
#define RED           	 0xF800
#define MAGENTA       	 0xF81F
#define GREEN         	 0x07E0
#define CYAN          	 0x7FFF
#define YELLOW        	 0xFFE0
#define BROWN 			 0XBC40 //×ØÉ«
#define BRRED 			 0XFC07 //×ØºìÉ«
#define GRAY  			 0X8430 //»ÒÉ«
//GUIÑÕÉ«

#define DARKBLUE      	 0X01CF	//ÉîÀ¶É«
#define LIGHTBLUE      	 0X7D7C	//Ç³À¶É«  
#define GRAYBLUE       	 0X5458 //»ÒÀ¶É«
//ÒÔÉÏÈýÉ«ÎªPANELµÄÑÕÉ« 
 
#define LIGHTGREEN     	 0X841F //Ç³ÂÌÉ«
//#define LIGHTGRAY        0XEF5B //Ç³»ÒÉ«(PANNEL)
#define LGRAY 			 0XC618 //Ç³»ÒÉ«(PANNEL),´°Ìå±³¾°É«

#define LGRAYBLUE        0XA651 //Ç³»ÒÀ¶É«(ÖÐ¼ä²ãÑÕÉ«)
#define LBBLUE           0X2B12 //Ç³×ØÀ¶É«(Ñ¡ÔñÌõÄ¿µÄ·´É«)
__inline void setv(u32 base,u32 addr,u32 bit_cnt,u32 value,u32 offset)
{
    ioa(base,addr)&=(~(((1<<bit_cnt)-1)<<offset));
    ioa(base,addr)|=(value<<offset);
}

void LCD_DisplayOn(void);
void LCD_DisplayOff(void);
void LCD_Real_Clear(u16 Color);	 
//void LCD_SetCursor(u16 Xpos, u16 Ypos);

u16  LCD_ReadPoint(u16 x,u16 y); //¶Áµã
void LCD_DrawCircle(u16 x0,u16 y0,u8 r);
void LCD_DrawLine(u16 x1, u16 y1, u16 x2, u16 y2);
void LCD_DrawRectangle(u16 x1, u16 y1, u16 x2, u16 y2);		   
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 *color);
void LCD_ShowChar(u16 x,u16 y,u8 num,u8 size,u8 mode);//ÏÔÊ¾Ò»¸ö×Ö·û
void LCD_ShowNum(u16 x,u16 y,s32 num,u8 len,u8 size,u8 mode);  //ÏÔÊ¾Ò»¸öÊý×Ö
void LCD_Show2Num(u16 x,u16 y,u16 num,u8 len,u8 size,u8 mode);//ÏÔÊ¾2¸öÊý×Ö
void LCD_ShowString(u16 x,u16 y,const u8 *p);		 //ÏÔÊ¾Ò»¸ö×Ö·û´®,16×ÖÌå
u16 yan(u8 a,u8 b,u8 c);
void huaanniu(u16 x,u16 y,const u8* ming,u8 neizhi);
void huashubiao(u16 x,u16 y);
void LCD_Clear(u8 a,u8 b,u8 c,u8 a1,u8 b1,u8 c1);									    

u16 LCD_ReadReg(u8 LCD_Reg);
void huatu_Init(void);

void LCD_WriteRAM(u16 RGB_Code);
u16 LCD_ReadRAM(void);		   
u16 LCD_BGR2RGB(u16 c);
u32 mypow(u8 m,u8 n);


u16 POINT_COLOR = 0x0000,BACK_COLOR = 0xFFFF;  
u16 DeviceCode;	 


//ÐŽŒÄŽæÆ÷º¯Êý
 	
//ÐŽŒÄŽæÆ÷
// void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
// {	
// 	LCD_WR_REG(LCD_Reg);  
// 	LCD_WR_DATA(LCD_RegValue);	    		 
// }	   
//¶ÁŒÄŽæÆ÷
u16 LCD_ReadReg(u8 LCD_Reg)
{										   
	u16 t;
	LCD_WR_REG(LCD_Reg);  //ÐŽÈëÒª¶ÁµÄŒÄŽæÆ÷ºÅ  
	// GPIOB->CRL=0X88888888; //PB0-7  ÉÏÀ­ÊäÈë
	// GPIOB->CRH=0X88888888; //PB8-15 ÉÏÀ­ÊäÈë
	// GPIOB->ODR=0XFFFF;    //È«²¿Êä³öžß
	{
		int i=0;
		for(i=0;i<8;i++)
	    {
	        setv((u32)io_addr,0x6c,3,0,i<<2);//PD0~7 as output
	        setv((u32)io_addr,0x70,3,0,i<<2);//PD8~15 as output
	    }
	    DATAOUT(0XFFFF);
	}
	LCD_RS_H;
	LCD_CS_L;
	//¶ÁÈ¡ÊýŸÝ(¶ÁŒÄŽæÆ÷Ê±,²¢²»ÐèÒª¶Á2ŽÎ)
	LCD_RD_L;					   
	LCD_RD_H;
	t=DATAIN;  
	LCD_CS_H; 
  	{
		int i=0;
		for(i=0;i<8;i++)
	    {
	        setv((u32)io_addr,0x6c,3,1,i<<2);//PD0~7 as output
	        setv((u32)io_addr,0x70,3,1,i<<2);//PD8~15 as output
	    }
	    DATAOUT(0XFFFF);
	}
	// GPIOB->CRL=0X33333333; //PB0-7  ÉÏÀ­Êä³ö
	// GPIOB->CRH=0X33333333; //PB8-15 ÉÏÀ­Êä³ö
	// GPIOB->ODR=0XFFFF;    //È«²¿Êä³öžß
	return t;  
}   
//¿ªÊŒÐŽGRAM
void LCD_WriteRAM_Prepare(void)
{
	LCD_WR_REG(R34);
}	 
//LCDÐŽGRAM
void LCD_WriteRAM(u16 RGB_Code)
{							    
	LCD_WR_DATA(RGB_Code);//ÐŽÊ®ÁùÎ»GRAM
}
//ŽÓILI93xx¶Á³öµÄÊýŸÝÎªGBRžñÊœ£¬¶øÎÒÃÇÐŽÈëµÄÊ±ºòÎªRGBžñÊœ¡£
//Íš¹ýžÃº¯Êý×ª»»
//c:GBRžñÊœµÄÑÕÉ«Öµ
//·µ»ØÖµ£ºRGBžñÊœµÄÑÕÉ«Öµ
__inline void LCD_WR_REG(u8 data)
{ 
	LCD_RS_L;//ÐŽµØÖ· 
 	LCD_CS_L; 
	DATAOUT(data); 
	LCD_WR_L; 
	LCD_WR_H; 
 	LCD_CS_H;  
}
__inline void LCD_WriteReg(u8 LCD_Reg, u16 LCD_RegValue)
{	
	LCD_WR_REG(LCD_Reg);  
	LCD_WR_DATA(LCD_RegValue);	    		 
}
__inline void LCD_SetCursor(u16 Xpos, u16 Ypos)
{

		LCD_WriteReg(R32, Ypos);
		LCD_WriteReg(R33, Xpos);

}  
__inline void LCD_DrawPoint(u16 x,u16 y,u16 color)
{
	LCD_SetCursor(319-x,y);
	LCD_WriteReg(R34,color);
} 					
u16 LCD_BGR2RGB(u16 c)
{
	u16  r,g,b,rgb;   
	b=(c>>0)&0x1f;
	g=(c>>5)&0x3f;
	r=(c>>11)&0x1f;	 
	rgb=(b<<11)+(g<<5)+(r<<0);		 
	return(rgb);
}		 
//¶ÁÈ¡žöÄ³µãµÄÑÕÉ«Öµ	 
//x:0~239
//y:0~319
//·µ»ØÖµ:ŽËµãµÄÑÕÉ«
u16 LCD_ReadPoint(u16 x,u16 y)
{
	u16 t;	
	if(x>=LCD_W||y>=LCD_H)return 0;//³¬¹ýÁË·¶Î§,Ö±œÓ·µ»Ø		   
	LCD_SetCursor(x,y);
	LCD_WR_REG(R34);       //Ñ¡ÔñGRAMµØÖ· 
	// GPIOB->CRL=0X88888888; //PB0-7  ÉÏÀ­ÊäÈë
	// GPIOB->CRH=0X88888888; //PB8-15 ÉÏÀ­ÊäÈë
	// GPIOB->ODR=0XFFFF;     //È«²¿Êä³öžß
		{
		int i=0;
		for(i=0;i<8;i++)
	    {
	        setv((u32)io_addr,0x6c,3,0,i<<2);//PD0~7 as output
	        setv((u32)io_addr,0x70,3,0,i<<2);//PD8~15 as output
	    }
	    DATAOUT(0XFFFF);
	}
	LCD_RS_H;
	LCD_CS_L;
	//¶ÁÈ¡ÊýŸÝ(¶ÁGRAMÊ±,ÐèÒª¶Á2ŽÎ)
	LCD_RD_L;					   
 	LCD_RD_H;
 	//dummy READ
	LCD_RD_L;					   
 	LCD_RD_H;
	t=DATAIN;  
	LCD_CS_H;  
	{
		int i=0;
		for(i=0;i<8;i++)
	    {
	        setv((u32)io_addr,0x6c,3,1,i<<2);//PD0~7 as output
	        setv((u32)io_addr,0x70,3,1,i<<2);//PD8~15 as output
	    }
	    DATAOUT(0XFFFF);
	}
	// GPIOB->CRL=0X33333333; //PB0-7  ÉÏÀ­Êä³ö
	// GPIOB->CRH=0X33333333; //PB8-15 ÉÏÀ­Êä³ö
	// GPIOB->ODR=0XFFFF;    //È«²¿Êä³öžß  
	if(DeviceCode==0X4535||DeviceCode==0X4531||DeviceCode==0X8989||DeviceCode==0XB505)return t;//ÕâŒžÖÖICÖ±œÓ·µ»ØÑÕÉ«Öµ
	else return LCD_BGR2RGB(t);
}
//LCD¿ªÆôÏÔÊŸ
void LCD_DisplayOn(void)
{					   
	LCD_WriteReg(R7, 0x0173); //26ÍòÉ«ÏÔÊŸ¿ªÆô
}	 
//LCD¹Ø±ÕÏÔÊŸ
void LCD_DisplayOff(void)
{	   
	LCD_WriteReg(R7, 0x0);//¹Ø±ÕÏÔÊŸ 
}   
//ÉèÖÃ¹â±êÎ»ÖÃ
//Xpos:ºá×ø±ê
//Ypos:×Ý×ø±ê

//»­µã
//x:0~239
//y:0~319
//POINT_COLOR:ŽËµãµÄÑÕÉ«
	 
//³õÊŒ»¯lcd
//žÃ³õÊŒ»¯º¯Êý¿ÉÒÔ³õÊŒ»¯ž÷ÖÖILI93XXÒºŸ§,µ«ÊÇÆäËûº¯ÊýÊÇ»ùÓÚILI9320µÄ!!!
//ÔÚÆäËûÐÍºÅµÄÇý¶¯ÐŸÆ¬ÉÏÃ»ÓÐ²âÊÔ! 
void lcd_init(void)
{ 
	int i=0;
    for(i=0;i<8;i++)
    {
        setv((u32)io_addr,0x6c,3,1,i<<2);//PD0~7 as output
        setv((u32)io_addr,0x70,3,1,i<<2);//PD8~15 as output
        DATAOUT(0XFFFF);
    }
    io_clear(io_addr,0x8c,0xffff);
    io_clear(io_addr,0x88,0xffff);
    setv((u32)io_addr,0x74,3,1,4);//D17
    setv((u32)io_addr,0x74,3,1,8);//D18
    setv((u32)io_addr,0x74,3,1,16);//D20
    setv((u32)io_addr,0x74,3,1,12);//D19
    setv((u32)io_addr,0x74,3,1,28);//D23
    LCD_RST_H;
	mdelay(50); 
	LCD_RST_L;
	mdelay(50);
	LCD_RST_H;
	mdelay(50); // delay 50 ms 
	LCD_WriteReg(0x0000,0x0001);
	mdelay(50); // delay 50 ms 
	DeviceCode = LCD_ReadReg(0x0000);   
	printk(" LCD ID:%x\n",DeviceCode); //ŽòÓ¡LCD ID 
	DeviceCode=0X4535;
	if(DeviceCode==0x9325||DeviceCode==0x9328)//ILI9325
	{
  		LCD_WriteReg(0x00e7,0x0010);      
        LCD_WriteReg(0x0000,0x0001);//¿ªÆôÄÚ²¿Ê±ÖÓ
        LCD_WriteReg(0x0001,0x0100);     
        LCD_WriteReg(0x0002,0x0700);//µçÔŽ¿ªÆô                    
		//LCD_WriteReg(0x0003,(1<<3)|(1<<4) ); 	//65K  RGB
		//DRIVE TABLE(ŒÄŽæÆ÷ 03H)
		//BIT3=AM BIT4:5=ID0:1
		//AM ID0 ID1   FUNCATION
		// 0  0   0	   R->L D->U
		// 1  0   0	   D->U	R->L
		// 0  1   0	   L->R D->U
		// 1  1   0    D->U	L->R
		// 0  0   1	   R->L U->D
		// 1  0   1    U->D	R->L
		// 0  1   1    L->R U->D Õý³£ŸÍÓÃÕâžö.
		// 1  1   1	   U->D	L->R
        LCD_WriteReg(0x0003,(1<<12)|(3<<4)|(0<<3) );//65K    
        LCD_WriteReg(0x0004,0x0000);                                   
        LCD_WriteReg(0x0008,0x0207);	           
        LCD_WriteReg(0x0009,0x0000);         
        LCD_WriteReg(0x000a,0x0000);//display setting         
        LCD_WriteReg(0x000c,0x0001);//display setting          
        LCD_WriteReg(0x000d,0x0000);//0f3c          
        LCD_WriteReg(0x000f,0x0000);
		//µçÔŽÅäÖÃ
        LCD_WriteReg(0x0010,0x0000);   
        LCD_WriteReg(0x0011,0x0007);
        LCD_WriteReg(0x0012,0x0000);                                                                 
        LCD_WriteReg(0x0013,0x0000);                 
        mdelay(50); 
        LCD_WriteReg(0x0010,0x1590);   
        LCD_WriteReg(0x0011,0x0227);
        mdelay(50); 
        LCD_WriteReg(0x0012,0x009c);                  
        mdelay(50); 
        LCD_WriteReg(0x0013,0x1900);   
        LCD_WriteReg(0x0029,0x0023);
        LCD_WriteReg(0x002b,0x000e);
        mdelay(50); 
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x013f);           
		mdelay(50); 
		//Ù€ÂíÐ£Õý
        LCD_WriteReg(0x0030,0x0007); 
        LCD_WriteReg(0x0031,0x0707);   
        LCD_WriteReg(0x0032,0x0006);
        LCD_WriteReg(0x0035,0x0704);
        LCD_WriteReg(0x0036,0x1f04); 
        LCD_WriteReg(0x0037,0x0004);
        LCD_WriteReg(0x0038,0x0000);        
        LCD_WriteReg(0x0039,0x0706);     
        LCD_WriteReg(0x003c,0x0701);
        LCD_WriteReg(0x003d,0x000f);
        mdelay(50); 
        LCD_WriteReg(0x0050,0x0000); //Ë®ÆœGRAMÆðÊŒÎ»ÖÃ 
        LCD_WriteReg(0x0051,0x00ef); //Ë®ÆœGRAMÖÕÖ¹Î»ÖÃ                    
        LCD_WriteReg(0x0052,0x0000); //Ž¹Ö±GRAMÆðÊŒÎ»ÖÃ                    
        LCD_WriteReg(0x0053,0x013f); //Ž¹Ö±GRAMÖÕÖ¹Î»ÖÃ  
        
        LCD_WriteReg(0x0060,0xa700);        
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006a,0x0000);
        LCD_WriteReg(0x0080,0x0000);
        LCD_WriteReg(0x0081,0x0000);
        LCD_WriteReg(0x0082,0x0000);
        LCD_WriteReg(0x0083,0x0000);
        LCD_WriteReg(0x0084,0x0000);
        LCD_WriteReg(0x0085,0x0000);
      
        LCD_WriteReg(0x0090,0x0010);     
        LCD_WriteReg(0x0092,0x0000);  
        LCD_WriteReg(0x0093,0x0003);
        LCD_WriteReg(0x0095,0x0110);
        LCD_WriteReg(0x0097,0x0000);        
        LCD_WriteReg(0x0098,0x0000);  
        //¿ªÆôÏÔÊŸÉèÖÃ    
        LCD_WriteReg(0x0007,0x0133);   
        LCD_WriteReg(0x0020,0x0000);                                                            
        LCD_WriteReg(0x0021,0x013f);
	}else if(DeviceCode==0x9320||DeviceCode==0x9300)
	{
		LCD_WriteReg(0x00,0x0000);
		LCD_WriteReg(0x01,0x0100);	//Driver Output Contral.
		LCD_WriteReg(0x02,0x0700);	//LCD Driver Waveform Contral.
		LCD_WriteReg(0x03,0x1030);//Entry Mode Set.
		//LCD_WriteReg(0x03,0x1018);	//Entry Mode Set.
	
		LCD_WriteReg(0x04,0x0000);	//Scalling Contral.
		LCD_WriteReg(0x08,0x0202);	//Display Contral 2.(0x0207)
		LCD_WriteReg(0x09,0x0000);	//Display Contral 3.(0x0000)
		LCD_WriteReg(0x0a,0x0000);	//Frame Cycle Contal.(0x0000)
		LCD_WriteReg(0x0c,(1<<0));	//Extern Display Interface Contral 1.(0x0000)
		LCD_WriteReg(0x0d,0x0000);	//Frame Maker Position.
		LCD_WriteReg(0x0f,0x0000);	//Extern Display Interface Contral 2.	    
		mdelay(50); 
		LCD_WriteReg(0x07,0x0101);	//Display Contral.
		mdelay(50); 								  
		LCD_WriteReg(0x10,(1<<12)|(0<<8)|(1<<7)|(1<<6)|(0<<4));	//Power Control 1.(0x16b0)
		LCD_WriteReg(0x11,0x0007);								//Power Control 2.(0x0001)
		LCD_WriteReg(0x12,(1<<8)|(1<<4)|(0<<0));				//Power Control 3.(0x0138)
		LCD_WriteReg(0x13,0x0b00);								//Power Control 4.
		LCD_WriteReg(0x29,0x0000);								//Power Control 7.
	
		LCD_WriteReg(0x2b,(1<<14)|(1<<4));	    
		LCD_WriteReg(0x50,0);	//Set X Star
		//Ë®ÆœGRAMÖÕÖ¹Î»ÖÃSet X End.
		LCD_WriteReg(0x51,239);	//Set Y Star
		LCD_WriteReg(0x52,0);	//Set Y End.t.
		LCD_WriteReg(0x53,319);	//
	
		LCD_WriteReg(0x60,0x2700);	//Driver Output Control.
		LCD_WriteReg(0x61,0x0001);	//Driver Output Control.
		LCD_WriteReg(0x6a,0x0000);	//Vertical Srcoll Control.
	
		LCD_WriteReg(0x80,0x0000);	//Display Position? Partial Display 1.
		LCD_WriteReg(0x81,0x0000);	//RAM Address Start? Partial Display 1.
		LCD_WriteReg(0x82,0x0000);	//RAM Address End-Partial Display 1.
		LCD_WriteReg(0x83,0x0000);	//Displsy Position? Partial Display 2.
		LCD_WriteReg(0x84,0x0000);	//RAM Address Start? Partial Display 2.
		LCD_WriteReg(0x85,0x0000);	//RAM Address End? Partial Display 2.
	
		LCD_WriteReg(0x90,(0<<7)|(16<<0));	//Frame Cycle Contral.(0x0013)
		LCD_WriteReg(0x92,0x0000);	//Panel Interface Contral 2.(0x0000)
		LCD_WriteReg(0x93,0x0001);	//Panel Interface Contral 3.
		LCD_WriteReg(0x95,0x0110);	//Frame Cycle Contral.(0x0110)
		LCD_WriteReg(0x97,(0<<8));	//
		LCD_WriteReg(0x98,0x0000);	//Frame Cycle Contral.	   
		LCD_WriteReg(0x07,0x0173);	//(0x0173)
	}else if(DeviceCode==0x5408)
	{
		LCD_WriteReg(0x01,0x0100);								  
		LCD_WriteReg(0x02,0x0700);//LCD Driving Waveform Contral 
		LCD_WriteReg(0x03,0x1030);//Entry ModeÉèÖÃ 	   
		//ÖžÕëŽÓ×óÖÁÓÒ×ÔÉÏ¶øÏÂµÄ×Ô¶¯ÔöÄ£Êœ
		//Normal Mode(Window Mode disable)
		//RGBžñÊœ
		//16Î»ÊýŸÝ2ŽÎŽ«ÊäµÄ8×ÜÏßÉèÖÃ
		LCD_WriteReg(0x04,0x0000); //Scalling Control register     
		LCD_WriteReg(0x08,0x0207); //Display Control 2 
		LCD_WriteReg(0x09,0x0000); //Display Control 3	 
		LCD_WriteReg(0x0A,0x0000); //Frame Cycle Control	 
		LCD_WriteReg(0x0C,0x0000); //External Display Interface Control 1 
		LCD_WriteReg(0x0D,0x0000); //Frame Maker Position		 
		LCD_WriteReg(0x0F,0x0000); //External Display Interface Control 2 
 		mdelay(20);
		//TFT ÒºŸ§²ÊÉ«ÍŒÏñÏÔÊŸ·œ·š14
		LCD_WriteReg(0x10,0x16B0); //0x14B0 //Power Control 1
		LCD_WriteReg(0x11,0x0001); //0x0007 //Power Control 2
		LCD_WriteReg(0x17,0x0001); //0x0000 //Power Control 3
		LCD_WriteReg(0x12,0x0138); //0x013B //Power Control 4
		LCD_WriteReg(0x13,0x0800); //0x0800 //Power Control 5
		LCD_WriteReg(0x29,0x0009); //NVM read data 2
		LCD_WriteReg(0x2a,0x0009); //NVM read data 3
		LCD_WriteReg(0xa4,0x0000);	 
		LCD_WriteReg(0x50,0x0000); //ÉèÖÃ²Ù×÷Ž°¿ÚµÄXÖá¿ªÊŒÁÐ
		LCD_WriteReg(0x51,0x00EF); //ÉèÖÃ²Ù×÷Ž°¿ÚµÄXÖáœáÊøÁÐ
		LCD_WriteReg(0x52,0x0000); //ÉèÖÃ²Ù×÷Ž°¿ÚµÄYÖá¿ªÊŒÐÐ
		LCD_WriteReg(0x53,0x013F); //ÉèÖÃ²Ù×÷Ž°¿ÚµÄYÖáœáÊøÐÐ
		LCD_WriteReg(0x60,0x2700); //Driver Output Control
		//ÉèÖÃÆÁÄ»µÄµãÊýÒÔŒ°ÉšÃèµÄÆðÊŒÐÐ
		LCD_WriteReg(0x61,0x0001); //Driver Output Control
		LCD_WriteReg(0x6A,0x0000); //Vertical Scroll Control
		LCD_WriteReg(0x80,0x0000); //Display Position šC Partial Display 1
		LCD_WriteReg(0x81,0x0000); //RAM Address Start šC Partial Display 1
		LCD_WriteReg(0x82,0x0000); //RAM address End - Partial Display 1
		LCD_WriteReg(0x83,0x0000); //Display Position šC Partial Display 2
		LCD_WriteReg(0x84,0x0000); //RAM Address Start šC Partial Display 2
		LCD_WriteReg(0x85,0x0000); //RAM address End šC Partail Display2
		LCD_WriteReg(0x90,0x0013); //Frame Cycle Control
		LCD_WriteReg(0x92,0x0000);  //Panel Interface Control 2
		LCD_WriteReg(0x93,0x0003); //Panel Interface control 3
		LCD_WriteReg(0x95,0x0110);  //Frame Cycle Control
		LCD_WriteReg(0x07,0x0173);		 
		mdelay(50);
	}	
	else if(DeviceCode==0x1505)
	{
		// second release on 3/5  ,luminance is acceptable,water wave appear during camera preview
        LCD_WriteReg(0x0007,0x0000);
        mdelay(50); 
        LCD_WriteReg(0x0012,0x011C);//0x011A   why need to set several times?
        LCD_WriteReg(0x00A4,0x0001);//NVM	 
        LCD_WriteReg(0x0008,0x000F);
        LCD_WriteReg(0x000A,0x0008);
        LCD_WriteReg(0x000D,0x0008);	    
  		//Ù€ÂíÐ£Õý
        LCD_WriteReg(0x0030,0x0707);
        LCD_WriteReg(0x0031,0x0007); //0x0707
        LCD_WriteReg(0x0032,0x0603); 
        LCD_WriteReg(0x0033,0x0700); 
        LCD_WriteReg(0x0034,0x0202); 
        LCD_WriteReg(0x0035,0x0002); //?0x0606
        LCD_WriteReg(0x0036,0x1F0F);
        LCD_WriteReg(0x0037,0x0707); //0x0f0f  0x0105
        LCD_WriteReg(0x0038,0x0000); 
        LCD_WriteReg(0x0039,0x0000); 
        LCD_WriteReg(0x003A,0x0707); 
        LCD_WriteReg(0x003B,0x0000); //0x0303
        LCD_WriteReg(0x003C,0x0007); //?0x0707
        LCD_WriteReg(0x003D,0x0000); //0x1313//0x1f08
        mdelay(50); 
        LCD_WriteReg(0x0007,0x0001);
        LCD_WriteReg(0x0017,0x0001);//¿ªÆôµçÔŽ
        mdelay(50); 
  		//µçÔŽÅäÖÃ
        LCD_WriteReg(0x0010,0x17A0); 
        LCD_WriteReg(0x0011,0x0217);//reference voltage VC[2:0]   Vciout = 1.00*Vcivl
        LCD_WriteReg(0x0012,0x011E);//0x011c  //Vreg1out = Vcilvl*1.80   is it the same as Vgama1out ?
        LCD_WriteReg(0x0013,0x0F00);//VDV[4:0]-->VCOM Amplitude VcomL = VcomH - Vcom Ampl
        LCD_WriteReg(0x002A,0x0000);  
        LCD_WriteReg(0x0029,0x000A);//0x0001F  Vcomh = VCM1[4:0]*Vreg1out    gate source voltage??
        LCD_WriteReg(0x0012,0x013E);// 0x013C  power supply on
        //Coordinates Control//
        LCD_WriteReg(0x0050,0x0000);//0x0e00
        LCD_WriteReg(0x0051,0x00EF); 
        LCD_WriteReg(0x0052,0x0000); 
        LCD_WriteReg(0x0053,0x013F); 
    	//Pannel Image Control//
        LCD_WriteReg(0x0060,0x2700); 
        LCD_WriteReg(0x0061,0x0001); 
        LCD_WriteReg(0x006A,0x0000); 
        LCD_WriteReg(0x0080,0x0000); 
    	//Partial Image Control//
        LCD_WriteReg(0x0081,0x0000); 
        LCD_WriteReg(0x0082,0x0000); 
        LCD_WriteReg(0x0083,0x0000); 
        LCD_WriteReg(0x0084,0x0000); 
        LCD_WriteReg(0x0085,0x0000); 
  		//Panel Interface Control//
        LCD_WriteReg(0x0090,0x0013);//0x0010 frenqucy
        LCD_WriteReg(0x0092,0x0300); 
        LCD_WriteReg(0x0093,0x0005); 
        LCD_WriteReg(0x0095,0x0000); 
        LCD_WriteReg(0x0097,0x0000); 
        LCD_WriteReg(0x0098,0x0000); 
  
        LCD_WriteReg(0x0001,0x0100); 
        LCD_WriteReg(0x0002,0x0700); 
        LCD_WriteReg(0x0003,0x1030); 
        LCD_WriteReg(0x0004,0x0000); 
        LCD_WriteReg(0x000C,0x0000); 
        LCD_WriteReg(0x000F,0x0000); 
        LCD_WriteReg(0x0020,0x0000); 
        LCD_WriteReg(0x0021,0x0000); 
        LCD_WriteReg(0x0007,0x0021); 
        mdelay(20);
        LCD_WriteReg(0x0007,0x0061); 
        mdelay(20);
        LCD_WriteReg(0x0007,0x0173); 
        mdelay(20);
	}else if(DeviceCode==0xB505)
	{
		LCD_WriteReg(0x0000,0x0000);
		LCD_WriteReg(0x0000,0x0000);
		LCD_WriteReg(0x0000,0x0000);
		LCD_WriteReg(0x0000,0x0000);
		
		LCD_WriteReg(0x00a4,0x0001);
		mdelay(20);		  
		LCD_WriteReg(0x0060,0x2700);
		LCD_WriteReg(0x0008,0x0202);
		
		LCD_WriteReg(0x0030,0x0214);
		LCD_WriteReg(0x0031,0x3715);
		LCD_WriteReg(0x0032,0x0604);
		LCD_WriteReg(0x0033,0x0e16);
		LCD_WriteReg(0x0034,0x2211);
		LCD_WriteReg(0x0035,0x1500);
		LCD_WriteReg(0x0036,0x8507);
		LCD_WriteReg(0x0037,0x1407);
		LCD_WriteReg(0x0038,0x1403);
		LCD_WriteReg(0x0039,0x0020);
		
		LCD_WriteReg(0x0090,0x001a);
		LCD_WriteReg(0x0010,0x0000);
		LCD_WriteReg(0x0011,0x0007);
		LCD_WriteReg(0x0012,0x0000);
		LCD_WriteReg(0x0013,0x0000);
		mdelay(20);
		
		LCD_WriteReg(0x0010,0x0730);
		LCD_WriteReg(0x0011,0x0137);
		mdelay(20);
		
		LCD_WriteReg(0x0012,0x01b8);
		mdelay(20);
		
		LCD_WriteReg(0x0013,0x0f00);
		LCD_WriteReg(0x002a,0x0080);
		LCD_WriteReg(0x0029,0x0048);
		mdelay(20);
		
		LCD_WriteReg(0x0001,0x0100);
		LCD_WriteReg(0x0002,0x0700);
		LCD_WriteReg(0x0003,0x1230);
		LCD_WriteReg(0x0008,0x0202);
		LCD_WriteReg(0x000a,0x0000);
		LCD_WriteReg(0x000c,0x0000);
		LCD_WriteReg(0x000d,0x0000);
		LCD_WriteReg(0x000e,0x0030);
		LCD_WriteReg(0x0050,0x0000);
		LCD_WriteReg(0x0051,0x00ef);
		LCD_WriteReg(0x0052,0x0000);
		LCD_WriteReg(0x0053,0x013f);
		LCD_WriteReg(0x0060,0x2700);
		LCD_WriteReg(0x0061,0x0001);
		LCD_WriteReg(0x006a,0x0000);
		//LCD_WriteReg(0x0080,0x0000);
		//LCD_WriteReg(0x0081,0x0000);
		LCD_WriteReg(0x0090,0X0011);
		LCD_WriteReg(0x0092,0x0600);
		LCD_WriteReg(0x0093,0x0402);
		LCD_WriteReg(0x0094,0x0002);
		mdelay(20);
		
		LCD_WriteReg(0x0007,0x0001);
		mdelay(20);
		LCD_WriteReg(0x0007,0x0061);
		LCD_WriteReg(0x0007,0x0173);
		
		LCD_WriteReg(0x0020,0x0000);
		LCD_WriteReg(0x0021,0x0000);	  
		LCD_WriteReg(0x00,0x22);  
	}else if(DeviceCode==0x8989)
	{	   
		LCD_WriteReg(0x0000,0x0001);//Žò¿ªŸ§Õñ
    	LCD_WriteReg(0x0003,0xA8A4);//0xA8A4
    	LCD_WriteReg(0x000C,0x0000);    
    	LCD_WriteReg(0x000D,0x080C);   
    	LCD_WriteReg(0x000E,0x2B00);    
    	LCD_WriteReg(0x001E,0x00B0);    
    	LCD_WriteReg(0x0001,0x2B3F);//Çý¶¯Êä³ö¿ØÖÆ320*240  0x6B3F
    	LCD_WriteReg(0x0002,0x0600);
    	LCD_WriteReg(0x0010,0x0000);  
    	LCD_WriteReg(0x0011,0x6070); //¶šÒåÊýŸÝžñÊœ  16Î»É« 		ºáÆÁ 0x6058
    	LCD_WriteReg(0x0005,0x0000);  
    	LCD_WriteReg(0x0006,0x0000);  
    	LCD_WriteReg(0x0016,0xEF1C);  
    	LCD_WriteReg(0x0017,0x0003);  
    	LCD_WriteReg(0x0007,0x0233); //0x0233       
    	LCD_WriteReg(0x000B,0x0000);  
    	LCD_WriteReg(0x000F,0x0000); //ÉšÃè¿ªÊŒµØÖ·
    	LCD_WriteReg(0x0041,0x0000);  
    	LCD_WriteReg(0x0042,0x0000);  
    	LCD_WriteReg(0x0048,0x0000);  
    	LCD_WriteReg(0x0049,0x013F);  
    	LCD_WriteReg(0x004A,0x0000);  
    	LCD_WriteReg(0x004B,0x0000);  
    	LCD_WriteReg(0x0044,0xEF00);  
    	LCD_WriteReg(0x0045,0x0000);  
    	LCD_WriteReg(0x0046,0x013F);  
    	LCD_WriteReg(0x0030,0x0707);  
    	LCD_WriteReg(0x0031,0x0204);  
    	LCD_WriteReg(0x0032,0x0204);  
    	LCD_WriteReg(0x0033,0x0502);  
    	LCD_WriteReg(0x0034,0x0507);  
    	LCD_WriteReg(0x0035,0x0204);  
    	LCD_WriteReg(0x0036,0x0204);  
    	LCD_WriteReg(0x0037,0x0502);  
    	LCD_WriteReg(0x003A,0x0302);  
    	LCD_WriteReg(0x003B,0x0302);  
    	LCD_WriteReg(0x0023,0x0000);  
    	LCD_WriteReg(0x0024,0x0000);  
    	LCD_WriteReg(0x0025,0x8000);  
    	LCD_WriteReg(0x004f,0);        //ÐÐÊ×Ö·0
    	LCD_WriteReg(0x004e,0);        //ÁÐÊ×Ö·0
	}else if(DeviceCode==0x4531)
	{
		LCD_WriteReg(0X00,0X0001);   
		mdelay(10);   
		LCD_WriteReg(0X10,0X1628);   
		LCD_WriteReg(0X12,0X000e);//0x0006    
		LCD_WriteReg(0X13,0X0A39);   
		mdelay(10);   
		LCD_WriteReg(0X11,0X0040);   
		LCD_WriteReg(0X15,0X0050);   
		mdelay(10);   
		LCD_WriteReg(0X12,0X001e);//16    
		mdelay(10);   
		LCD_WriteReg(0X10,0X1620);   
		LCD_WriteReg(0X13,0X2A39);   
		mdelay(10);   
		LCD_WriteReg(0X01,0X0100);   
		LCD_WriteReg(0X02,0X0300);   
		LCD_WriteReg(0X03,0X1030);//žÄ±ä·œÏòµÄ   
		LCD_WriteReg(0X08,0X0202);   
		LCD_WriteReg(0X0A,0X0008);   
		LCD_WriteReg(0X30,0X0000);   
		LCD_WriteReg(0X31,0X0402);   
		LCD_WriteReg(0X32,0X0106);   
		LCD_WriteReg(0X33,0X0503);   
		LCD_WriteReg(0X34,0X0104);   
		LCD_WriteReg(0X35,0X0301);   
		LCD_WriteReg(0X36,0X0707);   
		LCD_WriteReg(0X37,0X0305);   
		LCD_WriteReg(0X38,0X0208);   
		LCD_WriteReg(0X39,0X0F0B);   
		LCD_WriteReg(0X41,0X0002);   
		LCD_WriteReg(0X60,0X2700);   
		LCD_WriteReg(0X61,0X0001);   
		LCD_WriteReg(0X90,0X0210);   
		LCD_WriteReg(0X92,0X010A);   
		LCD_WriteReg(0X93,0X0004);   
		LCD_WriteReg(0XA0,0X0100);   
		LCD_WriteReg(0X07,0X0001);   
		LCD_WriteReg(0X07,0X0021);   
		LCD_WriteReg(0X07,0X0023);   
		LCD_WriteReg(0X07,0X0033);   
		LCD_WriteReg(0X07,0X0133);   
		LCD_WriteReg(0XA0,0X0000); 
	}else if(DeviceCode==0x4535)
	{			      
		LCD_WriteReg(0X15,0X0030);   
		LCD_WriteReg(0X9A,0X0010);   
 		LCD_WriteReg(0X11,0X0020);   
 		LCD_WriteReg(0X10,0X3428);   
		LCD_WriteReg(0X12,0X0002);//16    
 		LCD_WriteReg(0X13,0X1038);   
		mdelay(40);   
		LCD_WriteReg(0X12,0X0012);//16    
		mdelay(40);   
  		LCD_WriteReg(0X10,0X3420);   
 		LCD_WriteReg(0X13,0X3038);   
		mdelay(70);   
		LCD_WriteReg(0X30,0X0000);   
		LCD_WriteReg(0X31,0X0402);   
		LCD_WriteReg(0X32,0X0307);   
		LCD_WriteReg(0X33,0X0304);   
		LCD_WriteReg(0X34,0X0004);   
		LCD_WriteReg(0X35,0X0401);   
		LCD_WriteReg(0X36,0X0707);   
		LCD_WriteReg(0X37,0X0305);   
		LCD_WriteReg(0X38,0X0610);   
		LCD_WriteReg(0X39,0X0610); 
		  
		LCD_WriteReg(0X01,0X0100);   
		LCD_WriteReg(0X02,0X0300);   
		LCD_WriteReg(0X03,0X1030);//žÄ±ä·œÏòµÄ   
		LCD_WriteReg(0X08,0X0808);   
		LCD_WriteReg(0X0A,0X0008);   
 		LCD_WriteReg(0X60,0X2700);   
		LCD_WriteReg(0X61,0X0001);   
		LCD_WriteReg(0X90,0X013E);   
		LCD_WriteReg(0X92,0X0100);   
		LCD_WriteReg(0X93,0X0100);   
 		LCD_WriteReg(0XA0,0X3000);   
 		LCD_WriteReg(0XA3,0X0010);   
		LCD_WriteReg(0X07,0X0001);   
		LCD_WriteReg(0X07,0X0021);   
		LCD_WriteReg(0X07,0X0023);   
		LCD_WriteReg(0X07,0X0033);   
		LCD_WriteReg(0X07,0X0133);   
	}			 
	//LCD_LED=1;//µãÁÁ±³¹â	 
	LCD_Real_Clear(RED);
}  		  
  
//ÇåÆÁº¯Êý
//Color:ÒªÇåÆÁµÄÌî³äÉ«
void LCD_Real_Clear(u16 Color)
{
	u32 index=0;      
	LCD_SetCursor(0x00,0x0000);//ÉèÖÃ¹â±êÎ»ÖÃ 
	LCD_WriteRAM_Prepare();     //¿ªÊŒÐŽÈëGRAM	 	  
	for(index=0;index<76800;index++)
	{
		LCD_WR_DATA(Color);    
	}
}  
//ÔÚÖž¶šÇøÓòÄÚÌî³äÖž¶šÑÕÉ«
//ÇøÓòŽóÐ¡:
//  (xend-xsta)*(yend-ysta)
void LCD_Fill(u16 xsta,u16 ysta,u16 xend,u16 yend,u16 *color)
{          
	u16 i,j;
	u16 xlen=0;
//#if USE_HORIZONTAL==1
	xlen=yend-ysta+1;	   
	for(i=xsta;i<=xend;i++)
	{
	 	LCD_SetCursor(i,ysta);      //ÉèÖÃ¹â±êÎ»ÖÃ 
		LCD_WriteRAM_Prepare();     //¿ªÊŒÐŽÈëGRAM	  
		for(j=0;j<xlen;j++)LCD_WR_DATA(color[j*xlen+(i-xsta)]);//ÉèÖÃ¹â±êÎ»ÖÃ 	  
	}
// #else
// 	xlen=xend-xsta+1;	   
// 	for(i=ysta;i<=yend;i++)
// 	{
// 	 	LCD_SetCursor(xsta,i);      //ÉèÖÃ¹â±êÎ»ÖÃ 
// 		LCD_WriteRAM_Prepare();     //¿ªÊŒÐŽÈëGRAM	  
// 		for(j=0;j<xlen;j++)LCD_WR_DATA(color);//ÉèÖÃ¹â±êÎ»ÖÃ 	    
// 	}
// #endif						  	    
}  



