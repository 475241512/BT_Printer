#include "Type.h"
#include "Event.h"
#include "TP.h"
#include "basic_fun.h"
#include "Esc_p.h"
#include "ESC_POS_test.h"
#include "uart.h"


//======================================================================================================
static volatile uint8_t evtHead;
static uint8_t evtTail;
static uint8_t evtQueue[64];
//======================================================================================================

//======================================================================================================
extern void event_init(void)
{
	evtHead = evtTail = 0;
}
//======================================================================================================
extern void event_post(uint8_t event)
{
	uint32_t head;

	if(event)
	{
         // Disable Interrupts
		head = evtHead;
		evtHead = (head + 1) & (ARRAY_SIZE(evtQueue) - 1);
		// restore flag

		evtQueue[head] = event;
	}
}
//======================================================================================================
//======================================================================================================
extern uint8_t event_pend(void)
{
	uint8_t event;

	if(evtHead == evtTail)
	{
		return evtNULL;
	}
	event = evtQueue[evtTail];
	evtTail = (evtTail + 1) & (ARRAY_SIZE(evtQueue) - 1);
	return event;
}
//======================================================================================================
extern void event_proc(void)
{
	int i;
	switch(event_pend())
	{
	//-----------------------------------------------------------------
	case evtKeyDownFeed:
		//ESC_POS_test_esc();
		//ESC_POS_test_esc_special();
		//TPPrintTestPage();
#ifdef DEBUG_ESC_POS
		PrintCurrentBuffer(0);
#endif
		TPFeedStart();

          //if(TPPrinterReady())
          //{
          //  TPSelfTest2();
          //}
		
		break;
	case evtKeyUpFeed:
		TPFeedStop();
		break;
	case evtKeyDownHold500msMode:
        break;
	case evtKeyDownMode:
		#if 1
		if(TPPrinterReady())
        TPPrintTestPage();
		#endif
		break;
	case evtKeyDownHold2000msMode:
		if(TPPrinterReady())
        TPPrintTestPage();
		break;
	case evtKeyUpMode:
		break;
	case evtKeyHoldMode:
		break;
	case evtKeyDownHold5000msMode:
		break;
    case evtKeyDownHold7000msMode:
#ifdef LCD_VER 
		lcd_refresh_disable = 1;
		LCD_BACKLIGHT_ON();
		Lcd_clear(1);
		Lcd_TextOut(5,14,"Reset PIN");
        Lcd_TextOut(20,26,"...");
		BT_reset_PIN();
		need_update_bt_info_flag = 1;
		Lcd_disp_BT_info();
		delay_ms(2000);
		lcd_refresh_disable = 0;
#else
		LED_blink(10,100);
		BT_reset_PIN();
		if(TPPrinterReady())
			TPPrintTestPage();
#endif  
        break;
	case evtPaperOut:
		ESC_STS_STATUS_SET_FLAG(0x03,5);
		break;
	case evtPaperIn:
		ESC_STS_STATUS_RESET_FLAG(0x03,5);
        Wake_up();
        break;
    case evtBmDetect:
        break;
    case evtGetRealTimeStatus4:
       //Putchar(esc_sts.status4);
        break;
	case evtLifetest:
	   break;
	default:
		break;
	}
}
//======================================================================================================




