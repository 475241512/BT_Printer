#ifndef _HW_MISC_H_
#define _HW_MISC_H_
#include "stm32f10x_lib.h"
#include "Lcd.h"

#define HW_VER_V11	1
#define HW_VER_V12	2

//#define HW_VER		HW_VER_V11
#define HW_VER		HW_VER_V12

#define ENCRYPT_MODE_NONE		0		//应用没有加密
#define ENCRYPT_MODE_KT_XOR		1		//KT私有XOR加密方式
#define ENCRYPT_MODE_DES		2		//DES加密
#define ENCRYPT_MODE_3DES		3		//3DES加密

#define ENCRYPT_MODE	ENCRYPT_MODE_KT_XOR		//应用采取此加密方式，解密比较快，升级会比较快
//#define ENCRYPT_MODE	ENCRYPT_MODE_3DES		//应用采取此加密方式，解密比较慢，升级会比较慢

#define LED_ON()	GPIO_ResetBits(GPIOB, GPIO_Pin_12)
#define LED_OFF()	GPIO_SetBits(GPIOB, GPIO_Pin_12)

void LED_init(void);
void LED_blink(unsigned int cnt,unsigned int period_ms);
void LED_toggle(void);
void key_init(void);
int if_feed_key_preesed(void);
void delay_ms(unsigned int time);	
void platform_misc_port_init(void);
unsigned int hw_platform_USBcable_Insert_Detect(void);

#ifdef LCD_VER
extern void Lcd_process_bar_step(void);
#else
extern void LED_toggle(void);
#endif

#endif
