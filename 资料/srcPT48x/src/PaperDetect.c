#include"includes.h"

#define PAPER_SNS          (1ul<<0)
#define PAPER_READY        (1ul<<1)
#define BLACKMARKR_FLAG    (1ul<<3)
/*
#define AD_BLACKMARK_HIGH  (0X0300UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_HIGH)
*/

#if defined(PT723)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X00A0UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_LOW)
#elif defined(PT486)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_LOW)
#elif defined(PT48D)
#define AD_BLACKMARK_HIGH  (0X0200UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (0X00200UL)//无纸时一般是小于0xA0 1.5V
#else
#define AD_BLACKMARK_HIGH  (0X0300UL)
#define AD_BLACKMARK_LOW   (0X0050UL)
#define PAPER_AD_LTHRESHOLD  (AD_BLACKMARK_HIGH)
#endif

#define BMSNS()     ((((LPC_ADC->DR[0]>>6) & 0x3ff) < ((uint16_t)AD_BLACKMARK_HIGH))&&(((LPC_ADC->DR[0]>>6) & 0x3ff) > ((uint16_t)AD_BLACKMARK_LOW))?1:0)
#define PAPERSNS()  ((((LPC_ADC->DR[0]>>6) & 0x3ff)<PAPER_AD_LTHRESHOLD)?1:0)

#define Paper_SNS_GetStatus() (GPIO_ReadInputDataBit(PORT0,11))
uint8_t printersts,papercnt,platencnt,bm_cnt;

//======================================================================================================
#if 0
extern void TPPaperSNSInit(void)
{


    LPC_IOCON->R_PIO0_11=0x0C9;
    GPIOSetDir(PORT0,11,0);

    NVIC_SetPriority(TIMER_32_0_IRQn,2);
    init_timer32(0, 1000);
    LPC_TMR32B0->PR=PR_Val-1;//PR=31,1uS
#if defined(PT486) || defined(PT487)
    if(Paper_SNS_GetStatus()) 	// hardware revert
#elif defined(PT488)
    if(!Paper_SNS_GetStatus())
#endif
	 {
	  printersts |= ((1<<0) | (1<<1));
	 }
	else
	 {
	  printersts &= ~((1<<0) | (1<<1));
	 }
	  papercnt = 0;
      enable_timer32(0);
}
#else
extern void TPPaperSNSInit(void)
{
    LPC_SYSCON->PDRUNCFG &= ~(0x1<<4);//ADC??
    LPC_SYSCON->SYSAHBCLKCTRL |= (1<<13);//??ADC??

    LPC_IOCON->R_PIO0_11=0x02;
#if defined(TEMP_SNS_ENABLE)
    LPC_IOCON->R_PIO1_0=0x02;//配置为AD1，模拟输入
#endif

 #if defined(TP_VOLTAGE_SNS)
    LPC_IOCON->PIO1_11=0x01;//AD7
 #endif

 #if defined(TP_VOLTAGE_SNS)
    #if defined(TEMP_SNS_ENABLE)
	LPC_ADC->CR= ( (1<<AD0)|(1<<AD1)|(1<<AD7) | HardwareMode | ADCCLKDIV | ADCCLKS | STARTMode );
    #else
    LPC_ADC->CR= ( (1<<AD0) | (1<<AD7) | HardwareMode | ADCCLKDIV | ADCCLKS | STARTMode );
	#endif
#else
    #if defined(TEMP_SNS_ENABLE)
	LPC_ADC->CR= ( (1<<AD0)|(1<<AD1) | HardwareMode | ADCCLKDIV | ADCCLKS | STARTMode );
    #else
    LPC_ADC->CR= ( (1<<AD0) | HardwareMode | ADCCLKDIV | ADCCLKS | STARTMode );
	#endif
#endif

   NVIC_SetPriority(TIMER_32_0_IRQn,2);
   init_timer32(0, 10000);
   LPC_TMR32B0->PR=PR_Val-1;//PR=31,1uS

   esc_sts.status4=0;

   //enable_timer32(0);
   disable_timer32(0);
}

extern void PaperStartSns(void)
{
    #if defined(PT488)||defined(PT48D)//以下判断AD值的下限
            if(PAPERSNS()) //paper in
    #else
    		if(!PAPERSNS()) //
    #endif
       {
            printersts &= ~PAPER_SNS;
            //printersts &= ~PAPER_READY;
            printersts |= PAPER_READY;
			esc_sts.status4 &= ~(0x03<<5);


       }
       else // paper out
       {
            printersts |= PAPER_SNS;
            //printersts |= PAPER_READY;
            printersts &= ~PAPER_READY;
			esc_sts.status4 |= (0x03<<5);

       }

	enable_timer32(0);

}


#endif
extern void TPBMSNSDetect(void)
{
        if(BMSNS())         //?????????????1
        {
            if((printersts & (1 << 2)) == 0)        // previous head out
            {
                printersts |= (1<<2);
                bm_cnt = 1;  //10ms

            }
            else if(bm_cnt )
            {
                if((--bm_cnt ) == 0)
                {
                    if((printersts & BLACKMARKR_FLAG) == 0)
                    {
                        printersts |= BLACKMARKR_FLAG; // set head
                        event_post(evtBmDetect);
                    }
                }
            }
        }
        else
        {
            if(printersts & (1 << 2))   // previous head
            {
                printersts &= ~(1<<2);
                bm_cnt = 1;
            }
            else if(bm_cnt)
            {
                if((--bm_cnt) == 0)
                {
                    if((printersts & BLACKMARKR_FLAG))
                    {
                        printersts &= ~BLACKMARKR_FLAG;    // set head
                        event_post(evtBmNotDetect);
                    }
                }
            }

        }
}

extern void TPPaperSNSDetect(uint8_t c)//488?????,486?????
{

#if defined(PT486) || defined(PT487)
	if(!c)
#elif defined(PT488)||defined(PT48D)
    if(c)
#endif
	{
		if((printersts & (1 << 0)) == 0)		// previous paper out
		{
			printersts |= (1<<0);  //xxx1
			papercnt = 5;	// 50ms
		}
		else if(papercnt)
		{
			if((--papercnt) == 0)
			{
				if((printersts & (1 << 1)) == 0)
				{
					printersts |= PAPER_READY;	// set paper in //xx1x
					event_post(evtPaperIn);


				}
			}
		}
	}
	else
	{
		if(printersts & (1 << 0))	// previous paper in
		{
			printersts &= ~(1<<0);  //xxx0
			papercnt = 5;	// 50ms
		}
		else if(papercnt)
		{
			if((--papercnt) == 0)
			{
				if((printersts & (1 << 1)))
				{
					printersts &= ~(1 << 1);	// set paper out xx0x
					event_post(evtPaperOut);

				}
			}
		}
	}

}
//======================================================================================================
//=================================================================================================
extern uint8_t TPPrinterMark(void)
{
	if(printersts & BLACKMARKR_FLAG)
	{
         return FALSE;
    }
	else
	{
         return TRUE;
    }
}
//======================================================================================================

extern uint8_t TPPrinterReady(void)
{
   // return TRUE;

	if(printersts & PAPER_READY)
	{
		return TRUE;
	}
	else
    {
	    return FALSE;
	}

}
extern uint8_t TPPaperReady(void)
 {

	if(printersts & PAPER_READY)
	{
		return TRUE;
	}
	else
    {
	    return FALSE;
	}

}

//======================================================================================================
volatile uint16_t Power_AD;
static void PowerADFilter(void)
{
    static volatile uint16_t ad_buf[32];
    static unsigned int ad_sum=0,pt = 0;
    static int  first = 1;
    unsigned int new_val;
    new_val = ((LPC_ADC->DR[7]>>6) & 0x3ff);
    ad_sum += new_val;
    if(first)
    {
        if(pt == 31) first = 0;

    }
    else
    {
        ad_sum -= ad_buf[pt];
    }
    if(first)
    {
        Power_AD = (ad_sum /(pt+1));
    }
    else
    {
        Power_AD = (ad_sum /32);  //64个值的平均值
    }
    ad_buf[pt] = new_val;
    pt = (pt +1)&0x1f;


}
//================================================================================================
void TIMER32_0_IRQHandler(void)
{

  if ( LPC_TMR32B0->IR & 0x01 )
  {

  KeyScanProc();
  TPBMSNSDetect();
  //TPPaperSNSDetect( Paper_SNS_GetStatus());
  TPPaperSNSDetect( PAPERSNS());
  LedScanProc();
  PowerADFilter();
  LPC_TMR32B0->IR = 1;				//clear interrupt flag


  }
  if ( LPC_TMR32B0->IR & (0x1<<4) )
  {
	LPC_TMR32B0->IR = 0x1<<4;			// clear interrupt flag

  }
  return;
}







