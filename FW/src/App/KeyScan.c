#include "Type.h"
#include "KeyScan.h"
#include "Event.h"
#include "stm32f10x_lib.h"
#include "hw_platform.h"

#define KEY_MODE_DB_SHIFT		(1UL << 0)
#define KEY_MODE_SHIFT			(1UL << 1)
#define KEY_FEED_DB_SHIFT		(1UL << 2)
#define KEY_FEED_SHIFT			(1UL << 3)


static uint8_t keyStatus;
static uint16_t keyModeHold;

extern void KeyScanProc(void)
{
	unsigned char key;
	key = KEY_FEED();
	if (TPPaperReady() == 0)
	{
		key |= 0x10;
	}

	if(key&0x10)		// mode key up
	{
		//缺纸的情况下按进纸键
		if(key&0x01)
		{
			if((keyStatus & KEY_MODE_DB_SHIFT) == 0)	// status no changed
			{
				if(keyStatus & KEY_MODE_SHIFT)	// previous mode key up
				{
					event_post(evtKeyUpMode); // mode key up
					keyStatus &= ~KEY_MODE_SHIFT;
				}
			}
			else
			{
				keyStatus &= ~KEY_MODE_DB_SHIFT;//keyStatus = xxx0
			}
		}
		else
		{
			if(keyStatus & KEY_MODE_DB_SHIFT)	// status no changed
			{
				if((keyStatus & KEY_MODE_SHIFT) == 0)	// previous mode key down
				{
					event_post(evtKeyDownMode);	// mode key down
					keyModeHold = 700;			// 7 seconds
					keyStatus |= KEY_MODE_SHIFT;//
				}
				else if(keyModeHold)
				{
					keyModeHold--;
					switch(keyModeHold)
					{
					case 700-50:
						event_post(evtKeyDownHold500msMode);
						break;
					case 700-200:
						event_post(evtKeyDownHold2000msMode);
						break;
					case 700-500:
						event_post(evtKeyDownHold5000msMode);
						break;
					case 0:
						event_post(evtKeyDownHold7000msMode);
						break;
					}
				}
			}
			else
			{
				keyStatus |= KEY_MODE_DB_SHIFT;//keyStatus =xxx1
			}
		}
	}
	else
	{
		if(key&0x01)		// FEED key up
		{
			if((keyStatus & KEY_FEED_DB_SHIFT) == 0)	// status no changed
			{
				if(keyStatus & KEY_FEED_SHIFT)	// previous mode key up
				{
					event_post(evtKeyUpFeed); // mode key up
					keyStatus &= ~KEY_FEED_SHIFT;
				}
			}
			else
			{
				keyStatus &= ~KEY_FEED_DB_SHIFT;//keyStatus = xxx0
			}
		}
		else
		{
			if(keyStatus & KEY_FEED_DB_SHIFT)	// status no changed
			{
				if((keyStatus & KEY_FEED_SHIFT) == 0)	// previous mode key down
				{
					event_post(evtKeyDownFeed);	// mode key down
					keyStatus |= KEY_FEED_SHIFT;//
				}
			}
			else
			{
				keyStatus |= KEY_FEED_DB_SHIFT;//keyStatus =xxx1
			}
		}
	}

	//for burning test,auto post key event
	//     if (IsPrinterFree())
	//     {
	//event_post(evtKeyDownFeed);
	//     }

}
/*
static void PowerOnSelfTest(void)
{
	// two keys down
	//if((keyStatus & (KEY_FEED_SHIFT | KEY_MODE_SHIFT)) == (KEY_FEED_SHIFT | KEY_MODE_SHIFT))
	if((keyStatus & (KEY_FEED_SHIFT)) == (KEY_FEED_SHIFT ))
	{
		event_post(evtSelfTest);
	}
	systimeRemoveCallback(PowerOnSelfTest);
}
*/

//按键初始化
void KeyScanInit(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;

#if(HW_VER == HW_VER_V11)
	//FEED_KEY	-- PA.1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
#else
	//FEED_KEY	-- PE.2
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOE, &GPIO_InitStructure);

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
#endif
	//BOX_CTRL	-- PA.4
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);

	keyStatus = 0;
}


//开启钱箱控制的脉冲
void box_ctrl(int ms)
{
	GPIO_SetBits(GPIOA, GPIO_Pin_4);
	delay_ms(ms);
	GPIO_ResetBits(GPIOA, GPIO_Pin_4);
}




