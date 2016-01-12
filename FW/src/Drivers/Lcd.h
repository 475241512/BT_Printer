#ifndef _LCD_H_
#define _LCD_H_
#include "stm32f10x_lib.h"
#include "Terminal_Para.h"
#define LCD_BACKLIGHT_ON()	do{\
	GPIOB->BSRR = GPIO_Pin_12;\
	}while(0)

#define LCD_BACKLIGHT_OFF()	do{\
	GPIOB->BRR = GPIO_Pin_12;\
	}while(0)

extern unsigned char	lcd_refresh_disable;
extern unsigned char	need_update_bt_info_flag;
//È«ÆÁ
#define PIC_POWERON		1
#define PIC_NOPAPER		2

//°ëÆÁ
#define PIC_OK			3
#define PIC_FAIL		4
#define PIC_PIN_RESET	5
#define PIC_SYS_ERR		6

#define FONT_16x8		1
#define FONT_12x6		2

void Lcd_init(void);
void DispBmp1(void);
void Lcd_dispBMP(unsigned char	pic_id);
void Lcd_clear(unsigned char c);
void Lcd_Refresh(void);
void Lcd_DrawLineH(int x, int y, int w,unsigned short color);
void Lcd_DrawLineV(int x, int y, int h,unsigned short color);
void Lcd_TextOut(int x, int y, unsigned char *pText);
void Lcd_setfont(unsigned char font_size);
void Lcd_blink(unsigned int cnt,unsigned int period_ms);
void Lcd_disp_BT_info(void);
#endif