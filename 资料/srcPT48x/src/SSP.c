#include"includes.h"

void SSP_INIT(uint8_t ssp_num)
{
    if(ssp_num==0)
    {
        LPC_SYSCON->PRESETCTRL |=0x01;
        LPC_SYSCON->SYSAHBCLKCTRL |= (1<<11);
        LPC_SYSCON->SSP0CLKDIV=0x01;

 //       LPC_IOCON->PIO0_2 &= ~0x07;
 //       LPC_IOCON->PIO0_2 |= 0x01;	/* ssp0 SSEL0 */

        LPC_IOCON->SCK_LOC = 0x02; //SCK0 PIO0_6
        LPC_IOCON->PIO0_6 &= ~0x07;
        LPC_IOCON->PIO0_6 |= 0x12;  // ssp0 SCLK0

//        LPC_IOCON->PIO0_8 &= ~0x07;
//        LPC_IOCON->PIO0_8 |= 0x11;	/* ssp0 MISO0 */

        LPC_IOCON->PIO0_9 &= ~0x07;
        LPC_IOCON->PIO0_9 |= 0x11;  /* ssp0 MOSI0 */


        LPC_SSP0->CR0 =     (0x02 << 8) |                 /* SCR ����SPIʱ�ӷ�Ƶ */ //spiclk=pclk/(cpsr*(scr+1))
                            (0x00 << 7) |                 /* CPHA ʱ�������λ */
                                                   /* ��SPIģʽ��Ч */
                            (0x00 << 6) |                 /* CPOL ʱ���������, */
                                                   /* ��SPIģʽ��Ч */
                            (0x00 << 4) |                  /* FRF ֡��ʽ 00=SPI,01=SSI */
                                                    /* 10=Microwire,11=���� */
                            (0x07 << 0) ;                 /* DSS���ݳ���,0000-0010=���� */
                                                  /* 0011=4λ,0111=8λ, */
                                                    /* 1111=16λ */
        LPC_SSP0->CR1 =     (0x00 << 3) |                   /* SOD �ӻ��������,1=��ֹ */
                            (0x00 << 2) |                   /* MS ����ѡ��,0=����,1=�ӻ� */
                            (0x01 << 1) |                   /* SSE SSPʹ�� */
                            (0x00 << 0) ;                    /* LBM ��дģʽ,0Ϊ����ģʽ */
       LPC_SSP0->CPSR = 2 ;                               /* PCLK��Ƶֵ */
       LPC_SSP0->ICR = 0x03 ;                             /* �ж�����Ĵ��� */


    }

    if(ssp_num==1)
    {
        LPC_SYSCON->PRESETCTRL |=0x01;
        LPC_SYSCON->SYSAHBCLKCTRL |= (1<<18);
        LPC_SYSCON->SSP1CLKDIV=0x01;

        LPC_IOCON->PIO2_0 &= ~0x07;
        LPC_IOCON->PIO2_0 |= 0x02;  // ssp1 MOSI1

        LPC_IOCON->PIO2_1 &= ~0x07;
        LPC_IOCON->PIO2_1 |= 0x02;	/* ssp1 SCLK1 */

        LPC_IOCON->PIO2_2 &= ~0x07;
        LPC_IOCON->PIO2_2 |= 0x02;  // ssp1 MISO1

        LPC_IOCON->PIO2_3 &= ~0x07;
        LPC_IOCON->PIO2_3 |= 0x02;  // ssp1 MOSI1

        LPC_SSP1->CR0 =     (0x01 << 8) |                 /* SCR ����SPIʱ�ӷ�Ƶ */
                            (0x00 << 7) |                 /* CPHA ʱ�������λ */
                                                   /* ��SPIģʽ��Ч */
                            (0x00 << 6) |                 /* CPOL ʱ���������, */
                                                   /* ��SPIģʽ��Ч */
                            (0x00 << 4) |                  /* FRF ֡��ʽ 00=SPI,01=SSI */
                                                    /* 10=Microwire,11=���� */
                            (0x07 << 0) ;                 /* DSS���ݳ���,0000-0010=���� */
                                                  /* 0011=4λ,0111=8λ, */
                                                    /* 1111=16λ */
        LPC_SSP1->CR1 =     (0x00 << 3) |                   /* SOD �ӻ��������,1=��ֹ */
                            (0x00 << 2) |                   /* MS ����ѡ��,0=����,1=�ӻ� */
                            (0x01 << 1) |                   /* SSE SSPʹ�� */
                            (0x00 << 0) ;                    /* LBM ��дģʽ,0Ϊ����ģʽ */
       LPC_SSP1->CPSR = 2 ;                               /* PCLK��Ƶֵ */
       LPC_SSP1->ICR = 0x03 ;


    }

       return;

}
/*
void SSP_SendData(uint8_t SSP_num,uint8_t *BufferPtr)
{
    if(SSP_num==0)
    {
        while(! (LPC_SSP0->SR & SR_BSY));//wait for the last
        while(LPC_SSP0->SR & SR_TNF)
        {
            LPC_SSP0->DR = *BufferPtr;
        }
    }
   if(SSP_num==1)
   {
        while(! (LPC_SSP1->SR & SR_BSY));
        while(LPC_SSP1->SR & SR_TNF)
        {
            LPC_SSP1->DR = *BufferPtr;
        }
   }
}
*/
/***********************************************************************************************
** �������ƣ� SSP_SendData
** �������ܣ� SSP�ӿ���SPI���߷�������
** ��ڲ����� data �����͵�����
** ���ڲ����� ��ȡ������
***********************************************************************************************/
void SSP0_SendData (uint8_t data)
{

    while((LPC_SSP0->SR & SR_TFE) == 0);//����Ϊ��

    LPC_SSP0->DR = data;

    while((LPC_SSP0->SR & SR_BSY) == SR_BSY);//�ȴ��������


    return;

}




























