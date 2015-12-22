#include "Type.h"
#include "LED.h"

//==========================led=============================

extern void LedInit(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;

#if(HW_VER == HW_VER_V11)
	//LED_BLUE	-- PB.12
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_SetBits(GPIOB, GPIO_Pin_12);
#else
	//LED_BLUE	-- PD.14
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_14;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_SetBits(GPIOD, GPIO_Pin_14);
#endif
}

static int8_t led_flash_cnt;
#define LED_SLOW_FLASH_SPEED    4
#define LED_FAST_FLASH_SPEED    60

extern void LedScanProc(void)
{
	if(TPPrinterReady() == 0)	// 打印机未就绪
	{
	        if(TPPaperReady() == 0 )//缺纸
	        {
	            if(led_flash_cnt > 0)
	        	{
	        		if((--led_flash_cnt) == 0)
	        		{
	        			Led_Paper_OFF();
	        			led_flash_cnt = -LED_FAST_FLASH_SPEED;
	        		}
	        	}
	        	else if(led_flash_cnt < 0)
	        	{
	        		if((++led_flash_cnt) == 0)
	        		{
	        			Led_Paper_ON();
	        			led_flash_cnt = LED_FAST_FLASH_SPEED;
	        		}
	        	}
	        	else
	        	{
	        		Led_Paper_ON();
	        		led_flash_cnt = LED_FAST_FLASH_SPEED;
	        	}
	        }
		else
		{
			Led_Paper_OFF();
		}
	}
	else	// 打印机就绪
	{
		Led_Paper_OFF();
        led_flash_cnt = 0;
	}
}














