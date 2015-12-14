/****************************************************************************
 *   $Id:: uarttest.c 3635 2010-06-02 00:31:46Z usb00423                    $
 *   Project: NXP LPC11xx UART example
 *
 *   Description:
 *     This file contains UART test modules, main entry, to test UART APIs.
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


void Delay1ms(void)
{
    uint32_t ms=8000;
    while(ms--);
}


void Delayms(uint16_t ms)
{
    while(ms--)
    {
        Delay1ms();
    }
}
int main (void) {

  SystemInit();		//中断向量表的定位及系统时钟的初始化	
  LedInit();		//初始化LED指示灯
  TPInit();			//初始化打印头的控制模块
  TPPaperSNSInit(); //初始化缺纸检测及温度检测模块
  SSP_INIT(SSP0);	//打印头的SPI数据接口初始化
  F25L_Init();		//SPI FLASH接口初始化
  event_init();		//初始化事件环形缓冲区
  config_init();	//从SPI FLASH读取配置参数
  UARTInit(config_idx2baud(para.com_baud));	//根据配置的串口波特率初始化串口模块
  SetDesity();		//设置打印速度，表现为打印字体的密度
  esc_init();		//设置打印机控制板的工作参数
  KeyScanInit();	//按键检测模块的初始化
  Uart_RTS_LOW();	
  Delayms(100);
  PaperStartSns();	//检测缺纸，同时开启了定时器0，在定时器0的中断中按周期调用各种任务-- 按键处理、LED处理、缺纸检测、BMSNS、POWERAD

  while (1)
  {
     esc_p();		//等待串口接收到的数据，根据不同的数据进行相应的处理
  }
}
