#ifndef _LED_H
#define _LED_H
#include "stm32f10x_lib.h"
#include "hw_platform.h"

#if(HW_VER == HW_VER_V11)
#define LED_ON()     do{ \
           GPIO_ResetBits(GPIOB, GPIO_Pin_12); \
           }while(0)

#define LED_OFF()    do{ \
           GPIO_SetBits(GPIOB, GPIO_Pin_12); \
           }while(0)
#else
#define LED_ON()     do{ \
	GPIO_ResetBits(GPIOD, GPIO_Pin_14); \
		   }while(0)

#define LED_OFF()    do{ \
	GPIO_SetBits(GPIOD, GPIO_Pin_14); \
		   }while(0)
#endif

extern void LedInit(void);
extern void LedScanProc(void);
void LED_blink(unsigned int cnt,unsigned int period_ms);

#endif
