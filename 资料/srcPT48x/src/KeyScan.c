#include "includes.h"

#define KEY_MODE_DB_SHIFT		(1UL << 0)
#define KEY_MODE_SHIFT			(1UL << 1)
#define KEY_FEED_DB_SHIFT		(1UL << 2)
#define KEY_FEED_SHIFT			(1UL << 3)

#define KEY_MODE()  ( GPIO_ReadInputDataBit(PORT0,GPIO_Pin_1))
#define KEY_FEED()  (GPIO_ReadInputDataBit(PORT0,GPIO_Pin_4))



static uint8_t keyStatus;
static uint16_t keyModeHold;

extern void KeyScanProc(void)
{

	if(KEY_MODE())		// mode key up
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

	if(KEY_FEED())		// mode key up
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
extern void KeyScanInit(void)
{

    LPC_IOCON->PIO0_1=0x10;
    GPIOSetDir(PORT0,GPIO_Pin_1,Input);

	LPC_IOCON->PIO0_4=0x100;
    GPIOSetDir(PORT0,GPIO_Pin_4,Input);

    keyStatus = 0;

}





