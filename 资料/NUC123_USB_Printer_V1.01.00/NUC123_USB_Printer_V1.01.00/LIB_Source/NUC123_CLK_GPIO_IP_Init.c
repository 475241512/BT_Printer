#include  "NUC123Series.h"

void UnlockReg(void)
{
  while((SYS->REGWRPROT & 1) == 0){              // ��ֹ����Ͻ������ɹ�
    SYS->REGWRPROT = 0x59 ; 
    SYS->REGWRPROT = 0x16 ; 
    SYS->REGWRPROT = 0x88 ;
  }
} 
void  CLK_Init(void) 
{  
  UnlockReg() ;
    SYS->GPF_MFP = 3 ;                           // PF01 ���ó� XTAL ����
    CLK->PWRCON |= 1 ;	                         // ��HXT, 12MHz 
    while((CLK->CLKSTATUS & 0x01) == 0) ;        // ��HXT�ȶ�
    CLK->PLLCON  = 22 ;                          // 12M����Ƶ��(22+2)��Ƶ�� PLL=144MHz  
    CLK->CLKDIV  = 0x00010000 ;                  // ADCʱ�Ӷ���Ƶ
    CLK->CLKSEL1 = 0x00022050 ;                  // Timer12ѡHCLK, UART/ADC/PWM23 ѡ 12M
    CLK->CLKSEL2 = 0x08 ;	                       // FRQDIVѡHCLK 
    CLK->FRQDIV  = 0x12 ;                        // ���HCLK/8=9MHz,����110ns, ��֤HCLK 
    CLK->APBCLK  = 0x18217058 ;    // ʹ��ADC,USB,PWM,UART,FRQDIV,SPI,Timer12ʱ��
    CLK->APBDIV  = 0 ;                           // PCLK = HCLK 
    CLK->AHBCLK  = 2 ;                           // �� DMA ʱ��
    CLK->CLKDIV  = 0x21 ;                        // USB(2+1)��Ƶ,HCLK(1+1)��Ƶ
    while((CLK->CLKSTATUS & 0x04) == 0) ;        // ��PLL�ȶ�
    CLK->CLKSEL0 = 0x02 ;	                       // HCLKѡPLL, SysTickѡHXT = 12MHz
    CLK->PWRCON  = 0x119 ;                       // ʡ��ֻ��12M��10KRC, BOD��10KRC
  LOCKREG() ;
}                                      

//// NUC123 ����47��GPIO ////////////////////////////////////////////////////////////////
// 15��PB��û��, PD0123 4��Ҳû��
/////////////////////////////////////////////////////////////////////////////////////////
void  GPIO_Init(void) 
{                            // PA,6��, PA10~15
  PA->DOUT     = 0xFFFF ;                        // PA12_8.5Vʹ��,PA13_LATCH,���������1
  PA->PMD      = 0xF5FFFFFF ;                    // PA15_QB, PA12/13 ���ģʽ
  SYS->GPA_MFP = 0x4000 ;                        //PA10/11_SPI,PA12/13/15_GPIO,PA14_PWM_LED
                             // PB,15��, û��PB11
  PB->DOUT     = 0xFFFF ;                        // ��� 1
  PB->PMD      = 0xFFFFFFFF ;                    // ׼˫��
  SYS->GPB_MFP = 0 ;                             // GPIO
                             // PC12��: PC0~5,PC8~13
  PC->DOUT     = 0x00FF ;                        // PC8~13_OUT 0, STB 0 ��Ч
  PC->PMD      = 0x5555FFFF ;                    // PC8~PC13_���ģʽ, PC2׼˫��
  SYS->GPC_MFP = 0x003A ;                        // PC45_TxRx,PC1_CLK,PC3_MO
                             // PD10��: PD0~5,PD8~11
  PD->DOUT     = 0xFBFF ;                        // PD0123_GPIO, ׼˫�����1
  PD->PMD      = 0xFF17FFFF ;                    // PD9/10 ���ģʽ 1, PD11 ����ģʽ
  SYS->GPD_MFP = 0x00000130 ;                    // PD45_ADC, PD8_SPI
                             // PF4��: PF0~PF3
  PF->DOUT     = 0xFFFF ;                        // ��� 1
  PF->PMD      = 0xFFFFFF5F ;                    // PF2_MDA, PF3_MDB ���ģʽ
  SYS->GPF_MFP = 0x3 ;                           // PF01_XT, PF23_GPIO
  
  SYS->ALT_MFP = 0x60031800 ;
  SYS->ALT_MFP1= 0 ;
  
  PA->IMD  = 0x0000 ;                            // Edge interrupt mode
  PA->IEN  = 0x8000 ;                            //λ0~15�����ж�ʹ��,16~31����ʹ��
  PA->DBEN = 0x8000 ;                            //1 mean De-bounce
  GPIO->DBNCECON = 0x2B ;					               //De-bounce time = 2048HCLK

  NVIC_SetPriority(GPAB_IRQn, 2);                //Interrupt priority = 2 
  NVIC->ISER[0] = 1<<GPAB_IRQn ;		 				           
}
//// 1MHz ��������, ����ͨ�ö�ʱ, ����16.77��, �䶨ʱ�ж����ڿ�����ֽ ///////////////////
void Timer1Init(void)                                      // 72��Ƶ, 1us ����
{                                                         
  TIMER1->TCSR  = 0x1C810000 + 71 ;                        // ����ģʽ,(71+1)��Ƶ   
  NVIC->ISER[0] = 1 << TMR1_IRQn ;                         // ���ж�����
  TIMER1->TCSR |= 0x40000000 ;                             // ����Timer
  NVIC_SetPriority(TMR1_IRQn, 1) ;                         // �ж����ȼ�Ϊ1, ������ֽ��� 
}
void Delayus(uint32_t Tim)                                 // � 16777215 us 
{
  uint32_t T1 = TIMER1->TDR ;
  while(((TIMER1->TDR - T1)&0xFFFFFF) < Tim) ;
}
//// 8MHz ��������, ����ͨ�ö�ʱ, ����2.1��, �䶨ʱ�ж�����ֹͣ���� /////////////////////
void Timer2Init(void)                                      // 9��Ƶ, 0.125us����,���Ƽ���
{                                                         
  TIMER2->TCSR  = 0x1C810000 + 8 ;                         // ����ģʽ,���ж�,(8+1)��Ƶ   
  NVIC_SetPriority(TMR2_IRQn, 1) ;                         // �ж����ȼ�Ϊ1
  NVIC->ISER[0] = 1 << TMR2_IRQn ;                         // ���ж�����
  TIMER2->TCSR |= 0x40000000 ;                             // ����Timer
}
void DelayCLK(uint32_t Tim)                                   // 1 ��ʾ 1/8 ΢��
{                                                          // ����ִ��ʱ��,ʱ������0.5us
  uint32_t T0 = TIMER2->TDR ;                              
  while(((TIMER2->TDR - T0)&0xFFFFFF) < Tim) ;
}

void UART0Init(uint32_t Baud)
{
  UART0->FUN_SEL = 0 ;                                     // UART ģʽ
  UART0->LCR     = 3 ;                                     // 8λ����, 1-stop, no parity
  UART0->BAUD    = 0x30000000 + 12000000/Baud - 2 ;        // UART ʱ��ѡ���� 12M
  UART0->FCR     = 0x00030036 ;                            //14byte RTS��Ч,����8�ֽ��ж�
  UART0->TOR     = 250 ;                                   //250λʱ���޺�������, ��ʱ�ж�
  UART0->IER     = 3 ;                                     // ���շ��ж�
//   NVIC_SetPriority(UART0_IRQn, 3) ;                       // �ж����ȼ�Ϊ 3
//   NVIC->ISER[0] = 1 << UART0_IRQn ;                       // ���ж����� 
}
//// �� UART0 Tx FiFo ����,���ȷ��ͽ����ͷ���,FSR bit28�з��ͽ���,bit22��FiFo�� /////////
// �β� :  *pStr, �ַ�ָ��
//          Cnt , �ַ�����
// ���� :   pStr, ʣ������ַ���ַ, �������ַ���������, �򷵻� 0 
uint8_t* Tx0FillFiFo(uint8_t*pStr, uint32_t Cnt)
{
  if(Cnt){                                                 // ���ַ���Ҫ���
    do{
      if( UART0->FSR & 0x00800000 ) break ;                // FiFo���˾��˳�ѭ��        
      UART0->THR = *pStr++ ;                               // �� FiFo ���ַ�
    }while(--Cnt) ;                                        // ֱ�����ַ�
  }
  if(Cnt) return  pStr ; 
  else    return  0    ;                               
}
void PWM_Init(void)
{   
  PWMA->PPR  = 249<<8 ;                                    // PWM23ʱ��240��Ƶ, ��48KHz
  PWMA->CSR  = 0x3333 ;                                    // ��16��Ƶ, �� 3KHz
  PWMA->PCR  = 0x89090000 ;                                // PWM23 ���Ķ�������ģʽ
  PWMA->POE  = 0x0C ;                                      // PWM23 ���ʹ�� 
  PWMA->PIER = 0 ;                                         // ���ж�
  PWMA->CNR2 = 1500 ;                                      // ����ֵ, Լ 1 ��                                   
  PWMA->CMR2 = 1470 ;                                      // �͵�ƽʱ��(LED��)��
}
void ADCInit(void)
{
  ADC->ADCR  = 9 ;                                         // ʹ�ܵ���ɨ��ģʽ, ���ж�
}

// SPI1��Memory, DMA0�ܷ�, DMA1���� /////////////////////////////////////////////////////
void SPI_Init(void)                                     
{
  SPI0->CNTRL2  = 0x80000000 ;                             // ����Ƶ�ر�
  SPI0->DIVIDER = 14 ;                                     // Ptr: 72/(14+1) = 4.8M bps  
                                                           // ���1CLK, ������,��0x0400
  SPI1->CNTRL2  = 0x80000000 ;                      
  SPI1->DIVIDER = 4 ;                                      // MEM: 144/(4+1) = 28.8M bps  
                                                                        
  PDMA_GCR->GCRCSR |= 0x0700 ;                             // ��ͨ DMA012 ʱ��
  PDMA_GCR->PDSSR0  = 0xFFFF01FF ;                         // SPI1->TX:DMA0, SPI->RX:DMA1
                                      
  PDMA0->DAR  = (uint32_t)(&SPI1->TX[0]) ; 
  PDMA1->SAR  = (uint32_t)(&SPI1->RX[0]) ;                 // Դ��ַ: SPI ���� BUFF                                                                                                                                                                   
                                                      //MEM ��ַ,�����ֽ�BCR,����ǰ��ָ��       
  PDMA0->ISR  = ~0 ;                                       // �� 0 �жϱ�־
  PDMA2->ISR  = ~0 ;   PDMA3->ISR  = ~0 ;   
  PDMA4->ISR  = ~0 ;   PDMA5->ISR  = ~0 ;   
  PDMA0->IER  =  0 ;   PDMA1->IER  =  3 ;                  // PDMA ���ж�
  PDMA2->IER  =  0 ;   PDMA3->IER  =  0 ;                                      
  PDMA4->IER  =  0 ;   PDMA5->IER  =  0 ;                                        
  NVIC_SetPriority(PDMA_IRQn, 3) ;                         // �ж����ȼ�Ϊ3
  NVIC->ISER[0] = 1 << PDMA_IRQn ;                         // ���ж�����
}                                                                   
