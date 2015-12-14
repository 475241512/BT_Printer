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

  SystemInit();		//�ж�������Ķ�λ��ϵͳʱ�ӵĳ�ʼ��	
  LedInit();		//��ʼ��LEDָʾ��
  TPInit();			//��ʼ����ӡͷ�Ŀ���ģ��
  TPPaperSNSInit(); //��ʼ��ȱֽ��⼰�¶ȼ��ģ��
  SSP_INIT(SSP0);	//��ӡͷ��SPI���ݽӿڳ�ʼ��
  F25L_Init();		//SPI FLASH�ӿڳ�ʼ��
  event_init();		//��ʼ���¼����λ�����
  config_init();	//��SPI FLASH��ȡ���ò���
  UARTInit(config_idx2baud(para.com_baud));	//�������õĴ��ڲ����ʳ�ʼ������ģ��
  SetDesity();		//���ô�ӡ�ٶȣ�����Ϊ��ӡ������ܶ�
  esc_init();		//���ô�ӡ�����ư�Ĺ�������
  KeyScanInit();	//�������ģ��ĳ�ʼ��
  Uart_RTS_LOW();	
  Delayms(100);
  PaperStartSns();	//���ȱֽ��ͬʱ�����˶�ʱ��0���ڶ�ʱ��0���ж��а����ڵ��ø�������-- ��������LED����ȱֽ��⡢BMSNS��POWERAD

  while (1)
  {
     esc_p();		//�ȴ����ڽ��յ������ݣ����ݲ�ͬ�����ݽ�����Ӧ�Ĵ���
  }
}
