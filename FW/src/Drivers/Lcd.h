#ifndef _LCD_H_
#define _LCD_H_
#include "stm32f10x_lib.h"

#define LCD_BACKLIGHT_ON()	do{\
	GPIOB->BSRR = GPIO_Pin_12;\
	}while(0)

#define LCD_BACKLIGHT_OFF()	do{\
	GPIOB->BRR = GPIO_Pin_12;\
	}while(0)

void Lcd_init(void);
void DispBmp1(void);
void Lcd_dispLogo(void);
void Lcd_clear(unsigned char c);
void Lcd_Refresh(void);

#endif