/****************************************************************************
 *   $Id:: uart.c 3648 2010-06-02 21:41:06Z usb00423                        $
 *   Project: NXP LPC11xx UART example
 *
 *   Description:
 *     This file contains UART code example which include UART
 *     initialization, UART interrupt handler, and related APIs for
 *     UART access.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#include "includes.h"

#define UART_HW_FLOW
//#define UART_SF_FLOW//软流控
#define UART_TX_SUPPORT
#define UART_RX_SUPPORT

/********************************************/
static volatile uint16_t rxhead;
static uint16_t rxtail;
static uint8_t  rxbuf[UART_RX_BUF_SIZE];

/*********************************************/



volatile uint32_t UARTStatus;
volatile uint8_t  UARTTxEmpty = 1;
//volatile uint8_t  UARTBuffer[UART_RX_BUF_SIZE];
//volatile uint32_t UARTCount = 0;

//======================================================================================================
/*#if defined(UART_HW_FLOW)
#define RXBUF_HIGH_LEVEL		(sizeof(rxbuf)/4)
#define RXBUF_LOW_LEVEL			(sizeof(rxbuf)/8)
#endif*/
#ifdef  UART_SF_FLOW
uint8_t Com_Handshake_XON_flag  = ENABLE;
uint8_t Com_Handshake_XOFF_flag = ENABLE;
#endif
//======================================================================================================
#if defined(UART_HW_FLOW) ||  defined(UART_SF_FLOW)
extern uint16_t UartGetRxBufSize(void)
{
	uint16_t head;

	head = rxhead;
	if(head >= rxtail)
	{
		return ((sizeof(rxbuf)-1) - (head - rxtail));
	}
	else
	{
		return (rxtail - head - 1);
	}
}
#endif



/*****************************************************************************
** Function name:		UART_IRQHandler
**
** Descriptions:		UART interrupt handler
**
** parameters:			None
** Returned value:		None
**
*****************************************************************************/
void UART_IRQHandler(void)
{
  uint8_t IIRValue, LSRValue;
  uint8_t Dummy = Dummy;
  uint8_t ch;
  static  uint8_t pt=0;

  IIRValue = LPC_UART->IIR;

  IIRValue >>= 1;			/* skip pending bit in IIR */
  IIRValue &= 0x07;			/* check bit 1~3, interrupt identification */
  if (IIRValue == IIR_RLS)		/* Receive Line Status */
  {
    LSRValue = LPC_UART->LSR;
    /* Receive Line Status */
    if (LSRValue & (LSR_OE | LSR_PE | LSR_FE | LSR_RXFE | LSR_BI))
    {
      /* There are errors or break interrupt */
      /* Read LSR will clear the interrupt */
      UARTStatus = LSRValue;
      Dummy = LPC_UART->RBR;	/* Dummy read on RX to clear
								interrupt, then bail out */
      return;
    }
    if (LSRValue & LSR_RDR)	/* Receive Data Ready */
    {
      /* If no error on RLS, normal ready, save into the data buffer. */
      /* Note: read RBR will clear the interrupt */
//===================================================================
            ch =LPC_UART->RBR;

#if defined(FONT_DOWNLOAD)
        if(0==font_download)//字库包含10 04 n信
#endif
           	switch(pt)
			{
				case 0:
					if(ch == 0x10)//DLE
					{
						pt++;
					}
					break;
				case 1:
                    switch(ch)
                    {
                    case 0x04:
                        pt++;
                        break;
                    case 0x05:
                        pt+=2;
                        break;
                    case 0x10://容错处理:10 10 04 n
                        break;
                    default:
                        pt = 0;
                        break;
                    }
					break;
				case 2:
                    switch(ch)
                    {
                        case 4:
                            event_post(evtGetRealTimeStatus4);
                            break;
                        default:
                            break;
                    }
					pt = 0;
					break;
				case 3:
                    switch(ch)
                    {
                        case 1:
                            //错误下才执行
                            break;
                        case 2:
                             if(!TPPrinterReady())//错误下才执行
                                  WakeUpTP_MODE1();
                            break;
                        default:
                            break;
                    }
                    pt=0;
					break;
				default:
					pt = 0;
					break;
			}
        rxbuf[rxhead]= ch;
        rxhead = (rxhead + 1) & (sizeof(rxbuf) - 1);
 //===================================================================
 #if defined(UART_HW_FLOW)
     if(UartGetRxBufSize() < RXBUF_LOW_LEVEL)
	 {
	     Uart_RTS_HIGH();
	 }
 #elif defined(UART_SF_FLOW)
    if((UartGetRxBufSize() < RXBUF_LOW_LEVEL)&&(Com_Handshake_XOFF_flag ==ENABLE))
      {
              Putchar(XOFF);
              Com_Handshake_XOFF_flag =DISABLE;
              Com_Handshake_XON_flag =ENABLE;
      }
#endif
    }
  }
  else if (IIRValue == IIR_RDA)	/* Receive Data Available */
  {
    /* Receive Data Available */
 //===================================================================
        ch =LPC_UART->RBR;
#if defined(FONT_DOWNLOAD)
    if(0==font_download)
#endif
        switch(pt)
		{
			case 0:
				if(ch == 0x10)//DLE
				{
					pt++;
				}
				break;
			case 1:
                switch(ch)
                {
                case 0x04:
                    pt++;
                    break;
                case 0x05:
                    pt+=2;
                    break;
                case 0x10://容错处理:10 10 04 n
                    break;
                default:
                    pt = 0;
                    break;
                }
				break;
			case 2:
                switch(ch)
                {
                    case 4:
                        event_post(evtGetRealTimeStatus4);
                        break;
                    default:
                        break;
                }
				pt = 0;
				break;
			case 3:
                switch(ch)
                {
                    case 1:
                        //错误下才执行
                        break;
                    case 2:
                             if(!TPPrinterReady())//错误下才执行
                                  WakeUpTP_MODE1();
                        break;
                    default:
                        break;
                }
                pt=0;
				break;
			default:
				pt = 0;
				break;
		}
      rxbuf[rxhead]= ch;
      rxhead = (rxhead + 1) & (sizeof(rxbuf) - 1);
 //===================================================================
#if defined(UART_HW_FLOW)
      if(UartGetRxBufSize() < RXBUF_LOW_LEVEL)
       {
            Uart_RTS_HIGH();
       }
#elif defined(UART_SF_FLOW)
      if((UartGetRxBufSize() < RXBUF_LOW_LEVEL)&&(Com_Handshake_XOFF_flag ==ENABLE))
      {
            Putchar(XOFF);
            Com_Handshake_XOFF_flag =DISABLE;
            Com_Handshake_XON_flag =ENABLE;
      }
#endif


  }
  else if (IIRValue == IIR_CTI)	/* Character timeout indicator */
  {
    /* Character Time-out indicator */
    UARTStatus |= 0x100;		/* Bit 9 as the CTI error */
  }
  else if (IIRValue == IIR_THRE)	/* THRE, transmit holding register empty */
  {
    /* THRE interrupt */
    LSRValue = LPC_UART->LSR;		/* Check status in the LSR to see if
								valid data in U0THR or not */
    if (LSRValue & LSR_THRE)
    {
      UARTTxEmpty = 1;
    }
    else
    {
      UARTTxEmpty = 0;
    }
  }
  return;
}


/*****************************************************************************
** Function name:		UARTInit
**
** Descriptions:		Initialize UART0 port, setup pin select,
**				clock, parity, stop bits, FIFO, etc.
**
** parameters:			UART baudrate
** Returned value:		None
**
*****************************************************************************/
void UARTInit(uint32_t baudrate)
{
  uint32_t Fdiv;
  uint32_t regVal;

  UARTTxEmpty = 1;
  rxhead=rxtail=0;

  NVIC_DisableIRQ(UART_IRQn);

  LPC_IOCON->PIO1_6 &= ~0x07;    /*  UART I/O config */
  LPC_IOCON->PIO1_6 |= 0x01;     /* UART RXD */
  LPC_IOCON->PIO1_7 &= ~0x07;
  LPC_IOCON->PIO1_7 |= 0x01;     /* UART TXD */
//======================================================

  GPIOSetValue(PORT1,GPIO_Pin_5,0);
  LPC_IOCON->PIO1_5 &= ~0x07;
  LPC_IOCON->PIO1_5 |= 0x10;
  GPIOSetDir(PORT1,GPIO_Pin_5,Output);//RTS控制引脚

//======================================================
  /* Enable UART clock */
  LPC_SYSCON->SYSAHBCLKCTRL |= (1<<12);
  LPC_SYSCON->UARTCLKDIV = 0x1;     /* divided by 1 */

  LPC_UART->LCR = 0x83;             /* 8 bits, no Parity, 1 Stop bit ,DLAB=1 */
  regVal = LPC_SYSCON->UARTCLKDIV;
  Fdiv = ((SystemAHBFrequency/regVal)/16)/baudrate ;	/*baud rate */

  LPC_UART->DLM = Fdiv / 256;
  LPC_UART->DLL = Fdiv % 256;
  LPC_UART->LCR = 0x03;		/* DLAB = 0 */
  LPC_UART->FCR = 0x07;		/* Enable and reset TX and RX FIFO. */

  /* Read to clear the line status. */
  regVal = LPC_UART->LSR;

  /* Ensure a clean start, no data in either TX or RX FIFO. */
  while (( LPC_UART->LSR & (LSR_THRE|LSR_TEMT)) != (LSR_THRE|LSR_TEMT) );
  while ( LPC_UART->LSR & LSR_RDR )
  {
	regVal = LPC_UART->RBR;	/* Dump data from RX FIFO */
  }


   NVIC_SetPriority(UART_IRQn,0);
  /* Enable the UART Interrupt */
  NVIC_EnableIRQ(UART_IRQn);

  //Uart_RTS_LOW();

#if TX_INTERRUPT
  LPC_UART->IER = IER_RBR | IER_THRE | IER_RLS;	/* Enable UART interrupt */
#else
  LPC_UART->IER = IER_RBR | IER_RLS;	/* Enable UART interrupt */
#endif
  return;
}

/*****************************************************************************
** Function name:		UARTSend
**
** Descriptions:		Send a block of data to the UART 0 port based
**				on the data length
**
** parameters:		buffer pointer, and data length
** Returned value:	None
**
*****************************************************************************/
void UARTSend(uint8_t *BufferPtr, uint32_t Length)
{

  while ( Length != 0 )
  {
	  /* THRE status, contain valid data */
#if !TX_INTERRUPT
	  while ( !(LPC_UART->LSR & LSR_THRE) );
	  LPC_UART->THR = *BufferPtr;
#else
	  /* Below flag is set inside the interrupt handler when THRE occurs. */
      while ( !(UARTTxEmpty & 0x01) );
	  LPC_UART->THR = *BufferPtr;
      UARTTxEmpty = 0;	/* not empty in the THR until it shifts out */
#endif
      BufferPtr++;
      Length--;
  }
  return;
}
#if defined(UART_TX_SUPPORT)
/******************************************************************************
**Function name:  Putchar
**
**Description:   Send a block of data to the UART 0 port
**
**parameters: none
**Returned value:
**
******************************************************************************/
    extern uint8_t Putchar(uint8_t c)          //发送数据
    {
 //       while ( !(LPC_UART->LSR & LSR_THRE) );
//        LPC_UART->THR = c;

        UARTSend((uint8_t*)(&c),sizeof(c));
        return c;
    }

    #endif

    #if defined(UART_RX_SUPPORT)
/******************************************************************************
**Function name:  Getchar
**
**Description:  Get a char from the uart 0 port
**
**parameters: none
**Returned value:
**
******************************************************************************/
extern uint8_t Getchar(void)        //接收数据
    {
        uint8_t c;
        uint16_t head;
        while(1)
        {
            event_proc();

            head = rxhead;
            if(head != rxtail)
            {
                c = rxbuf[rxtail];
                rxtail = (rxtail + 1) & (sizeof(rxbuf) - 1);
#if defined(UART_HW_FLOW)
            if((UartGetRxBufSize() > RXBUF_HIGH_LEVEL))
			{
				Uart_RTS_LOW();
			}
#elif defined(UART_SF_FLOW)
          if((UartGetRxBufSize() > RXBUF_HIGH_LEVEL)&&(Com_Handshake_XON_flag == ENABLE))
			{
                Putchar(XON);
                Com_Handshake_XON_flag  = DISABLE;
                Com_Handshake_XOFF_flag = ENABLE;

			}

#endif
                return c;

            }
        //    event_proc();
        }

    }
 #endif
 extern uint8_t PrintBufGetchar(uint8_t *ch)
 {

    if(rxhead != rxtail)	// have data
    {
            *ch = rxbuf[rxtail];
            rxtail = (rxtail + 1) & (sizeof(rxbuf) - 1);
    #if defined(UART_HW_FLOW)
    	if((UartGetRxBufSize() > RXBUF_HIGH_LEVEL))
    	{
    		Uart_RTS_LOW();
    	}

    #elif defined(UART_SF_FLOW)
              if((UartGetRxBufSize() > RXBUF_HIGH_LEVEL)&&(Com_Handshake_XON_flag == ENABLE))
                {
                    Putchar(XON);
                    Com_Handshake_XON_flag  = DISABLE;
                    Com_Handshake_XOFF_flag = ENABLE;

                }

    #endif
            return 1;

     }
    else
        return 0;

 }

 extern void PrintBufPushBytes(uint8_t c)
 {
      rxbuf[rxhead]= c;
      rxhead = (rxhead + 1) & (sizeof(rxbuf) - 1);
 }

//======================================================================================================
extern void PrintBufPushLine( uint8_t *p, uint32_t len)
{
	uint32_t i;

	for(i=0;i<len;i++)
   {
     PrintBufPushBytes(*p++);
   }

}
extern void PrintBufToZero(void)
{
     rxhead=rxtail=0;
}
/******************************************************************************
**                            End Of File
******************************************************************************/
