#include "stm32f10x_it.h"
#include "usb_lib.h"
#include "usb_prop.h"
#include "usb_desc.h"
#include "hw_config.h"
#include "platform_config.h"
#include "usb_pwr.h"
#include "usb_ch341.h"
#include "usb_init.h"
#include "delay.h"
#include "lcd.h"
static void fill(const u8 *data,u8 lenth);
static void rect(const u8 *data,u8 lenth);
static void bitblt(const u8 *data,u8 lenth);
static void copyarea(const u8 *data,u8 lenth);
static u32 pix_index;
static u16 x_start;
static u16 y_start;
static u16 width;
static u16 height;
static u8 mode;
USART_InitTypeDef USART_InitStructure;
u16 ch341_baud;

u8 buffer_in[VIRTUAL_COM_PORT_DATA_SIZE];
extern  u32 count_in;
extern u16 POINT_COLOR;
__inline void draw_point(u8 color)
{
    static u32 color_pre;
    if(pix_index&1)//draw
    {
        POINT_COLOR|=color<<8;
        LCD_DrawPoint(319-x_start-pix_index/2%width,y_start+pix_index/2/width);
    }
    else
    {
        POINT_COLOR=color;
    }
}
void USB_CH341_Init()
{
    delay_ms(200);
    Set_USBClock();
	USB_Interrupts_Config();	
	USB_Init();	
    delay_ms(1000);
    delay_ms(1000);
}

void USB_Cable_Config(FunctionalState NewState)
{
  if (NewState != DISABLE)
  {
    GPIO_SetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
  else
  {
    GPIO_ResetBits(USB_DISCONNECT, USB_DISCONNECT_PIN);
  }
}

void USB_send(u8 *buf,u8 lenth)
{
  UserToPMABufferCopy(buf, ENDP1_TXADDR, lenth);
  SetEPTxCount(ENDP2, lenth);
  SetEPTxValid(ENDP2);
  while(GetEPTxStatus(2)!=32);
}
void USB_receive(const u8 *buf,u8 lenth)
{
    mode=buf[0]&0x3f;
    printf("mode=%d",mode);
    switch(mode)
    {
    case 1:
        fill(buf,lenth);
        break;
    case 2:
        bitblt(buf,lenth);
        break;
    case 3:
        rect(buf,lenth);
        break;
    case 4:
        copyarea(buf,lenth);
        break;
    }
    
}
static void fill(const u8 *data,u8 lenth)
{
    putchar('f');
    putchar('\r');
    putchar('\n'); 
}
static void rect(const u8 *data,u8 lenth)
{
    putchar('r');
    putchar('\r');
    putchar('\n'); 
}
#define mdbg(a) printf(#a"=%d",a)
static void bitblt(const u8 *data,u8 lenth)
{

    static u8 operation;
    u8 i=0;
    if(data[0]&0x80)
    {
        //putchar('b');
        //return ;
        x_start=(data[2]<<8)+data[1];
        y_start=(data[4]<<8)+data[3];
        width=(data[6]<<8)+data[5];
        height=(data[8]<<8)+data[7];
        operation=data[9];
//        mdbg(x_start);
//        mdbg(y_start);
//        mdbg(width);
//        mdbg(height);
//        mdbg(operation);
//        putchar('\r');
//        putchar('\n');  
        pix_index=0;
        for(i=12;i<lenth;i++)
        {
            draw_point(data[i]);
            pix_index++;
        }
            
    }
    else
    {
        //return ;
        for(i=1;i<lenth;i++)
        {
            draw_point(data[i]);
            pix_index++;
        }
            
    }
        
}

static void copyarea(const u8 *data,u8 lenth)
{
    putchar('c');
    putchar('\r');
    putchar('\n'); 
}

