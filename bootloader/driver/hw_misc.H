#ifndef _HW_MISC_H_
#define _HW_MISC_H_
#include "stm32f10x_lib.h"

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

#endif
