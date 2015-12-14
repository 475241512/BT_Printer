#include  "NUC123Series.h"

void UnlockReg(void)
{
  while((SYS->REGWRPROT & 1) == 0){              // 防止被打断解锁不成功
    SYS->REGWRPROT = 0x59 ; 
    SYS->REGWRPROT = 0x16 ; 
    SYS->REGWRPROT = 0x88 ;
  }
} 
void  CLK_Init(void) 
{  
  UnlockReg() ;
    SYS->GPF_MFP = 3 ;                           // PF01 配置成 XTAL 引脚
    CLK->PWRCON |= 1 ;	                         // 打开HXT, 12MHz 
    while((CLK->CLKSTATUS & 0x01) == 0) ;        // 等HXT稳定
    CLK->PLLCON  = 22 ;                          // 12M二分频再(22+2)倍频得 PLL=144MHz  
    CLK->CLKDIV  = 0x00010000 ;                  // ADC时钟二分频
    CLK->CLKSEL1 = 0x00022050 ;                  // Timer12选HCLK, UART/ADC/PWM23 选 12M
    CLK->CLKSEL2 = 0x08 ;	                       // FRQDIV选HCLK 
    CLK->FRQDIV  = 0x12 ;                        // 输出HCLK/8=9MHz,周期110ns, 验证HCLK 
    CLK->APBCLK  = 0x18217058 ;    // 使能ADC,USB,PWM,UART,FRQDIV,SPI,Timer12时钟
    CLK->APBDIV  = 0 ;                           // PCLK = HCLK 
    CLK->AHBCLK  = 2 ;                           // 打开 DMA 时钟
    CLK->CLKDIV  = 0x21 ;                        // USB(2+1)分频,HCLK(1+1)分频
    while((CLK->CLKSTATUS & 0x04) == 0) ;        // 等PLL稳定
    CLK->CLKSEL0 = 0x02 ;	                       // HCLK选PLL, SysTick选HXT = 12MHz
    CLK->PWRCON  = 0x119 ;                       // 省电只开12M和10KRC, BOD用10KRC
  LOCKREG() ;
}                                      

//// NUC123 共有47个GPIO ////////////////////////////////////////////////////////////////
// 15个PB还没用, PD0123 4个也没用
/////////////////////////////////////////////////////////////////////////////////////////
void  GPIO_Init(void) 
{                            // PA,6个, PA10~15
  PA->DOUT     = 0xFFFF ;                        // PA12_8.5V使能,PA13_LATCH,两个都输出1
  PA->PMD      = 0xF5FFFFFF ;                    // PA15_QB, PA12/13 输出模式
  SYS->GPA_MFP = 0x4000 ;                        //PA10/11_SPI,PA12/13/15_GPIO,PA14_PWM_LED
                             // PB,15个, 没有PB11
  PB->DOUT     = 0xFFFF ;                        // 输出 1
  PB->PMD      = 0xFFFFFFFF ;                    // 准双向
  SYS->GPB_MFP = 0 ;                             // GPIO
                             // PC12个: PC0~5,PC8~13
  PC->DOUT     = 0x00FF ;                        // PC8~13_OUT 0, STB 0 无效
  PC->PMD      = 0x5555FFFF ;                    // PC8~PC13_输出模式, PC2准双向
  SYS->GPC_MFP = 0x003A ;                        // PC45_TxRx,PC1_CLK,PC3_MO
                             // PD10个: PD0~5,PD8~11
  PD->DOUT     = 0xFBFF ;                        // PD0123_GPIO, 准双向输出1
  PD->PMD      = 0xFF17FFFF ;                    // PD9/10 输出模式 1, PD11 输入模式
  SYS->GPD_MFP = 0x00000130 ;                    // PD45_ADC, PD8_SPI
                             // PF4个: PF0~PF3
  PF->DOUT     = 0xFFFF ;                        // 输出 1
  PF->PMD      = 0xFFFFFF5F ;                    // PF2_MDA, PF3_MDB 输出模式
  SYS->GPF_MFP = 0x3 ;                           // PF01_XT, PF23_GPIO
  
  SYS->ALT_MFP = 0x60031800 ;
  SYS->ALT_MFP1= 0 ;
  
  PA->IMD  = 0x0000 ;                            // Edge interrupt mode
  PA->IEN  = 0x8000 ;                            //位0~15下沿中断使能,16~31上沿使能
  PA->DBEN = 0x8000 ;                            //1 mean De-bounce
  GPIO->DBNCECON = 0x2B ;					               //De-bounce time = 2048HCLK

  NVIC_SetPriority(GPAB_IRQn, 2);                //Interrupt priority = 2 
  NVIC->ISER[0] = 1<<GPAB_IRQn ;		 				           
}
//// 1MHz 连续计数, 用于通用定时, 周期16.77秒, 其定时中断用于控制走纸 ///////////////////
void Timer1Init(void)                                      // 72分频, 1us 计数
{                                                         
  TIMER1->TCSR  = 0x1C810000 + 71 ;                        // 连续模式,(71+1)分频   
  NVIC->ISER[0] = 1 << TMR1_IRQn ;                         // 打开中断向量
  TIMER1->TCSR |= 0x40000000 ;                             // 启动Timer
  NVIC_SetPriority(TMR1_IRQn, 1) ;                         // 中断优先级为1, 控制走纸电机 
}
void Delayus(uint32_t Tim)                                 // 最长 16777215 us 
{
  uint32_t T1 = TIMER1->TDR ;
  while(((TIMER1->TDR - T1)&0xFFFFFF) < Tim) ;
}
//// 8MHz 连续计数, 用于通用定时, 周期2.1秒, 其定时中断用于停止加热 /////////////////////
void Timer2Init(void)                                      // 9分频, 0.125us计数,控制加热
{                                                         
  TIMER2->TCSR  = 0x1C810000 + 8 ;                         // 连续模式,开中断,(8+1)分频   
  NVIC_SetPriority(TMR2_IRQn, 1) ;                         // 中断优先级为1
  NVIC->ISER[0] = 1 << TMR2_IRQn ;                         // 打开中断向量
  TIMER2->TCSR |= 0x40000000 ;                             // 启动Timer
}
void DelayCLK(uint32_t Tim)                                   // 1 表示 1/8 微秒
{                                                          // 代码执行时间,时长增加0.5us
  uint32_t T0 = TIMER2->TDR ;                              
  while(((TIMER2->TDR - T0)&0xFFFFFF) < Tim) ;
}

void UART0Init(uint32_t Baud)
{
  UART0->FUN_SEL = 0 ;                                     // UART 模式
  UART0->LCR     = 3 ;                                     // 8位数据, 1-stop, no parity
  UART0->BAUD    = 0x30000000 + 12000000/Baud - 2 ;        // UART 时钟选的是 12M
  UART0->FCR     = 0x00030036 ;                            //14byte RTS有效,多于8字节中断
  UART0->TOR     = 250 ;                                   //250位时间无后续数据, 超时中断
  UART0->IER     = 3 ;                                     // 开收发中断
//   NVIC_SetPriority(UART0_IRQn, 3) ;                       // 中断优先级为 3
//   NVIC->ISER[0] = 1 << UART0_IRQn ;                       // 打开中断向量 
}
//// 把 UART0 Tx FiFo 填满,不等发送结束就返回,FSR bit28判发送结束,bit22判FiFo空 /////////
// 形参 :  *pStr, 字符指针
//          Cnt , 字符个数
// 返回 :   pStr, 剩余的首字符地址, 若所有字符都输出完成, 则返回 0 
uint8_t* Tx0FillFiFo(uint8_t*pStr, uint32_t Cnt)
{
  if(Cnt){                                                 // 有字符需要输出
    do{
      if( UART0->FSR & 0x00800000 ) break ;                // FiFo满了就退出循环        
      UART0->THR = *pStr++ ;                               // 往 FiFo 填字符
    }while(--Cnt) ;                                        // 直到无字符
  }
  if(Cnt) return  pStr ; 
  else    return  0    ;                               
}
void PWM_Init(void)
{   
  PWMA->PPR  = 249<<8 ;                                    // PWM23时钟240分频, 得48KHz
  PWMA->CSR  = 0x3333 ;                                    // 再16分频, 得 3KHz
  PWMA->PCR  = 0x89090000 ;                                // PWM23 中心对齐周期模式
  PWMA->POE  = 0x0C ;                                      // PWM23 输出使能 
  PWMA->PIER = 0 ;                                         // 关中断
  PWMA->CNR2 = 1500 ;                                      // 周期值, 约 1 秒                                   
  PWMA->CMR2 = 1470 ;                                      // 低电平时间(LED亮)短
}
void ADCInit(void)
{
  ADC->ADCR  = 9 ;                                         // 使能单次扫描模式, 关中断
}

// SPI1接Memory, DMA0管发, DMA1管收 /////////////////////////////////////////////////////
void SPI_Init(void)                                     
{
  SPI0->CNTRL2  = 0x80000000 ;                             // 二分频关闭
  SPI0->DIVIDER = 14 ;                                     // Ptr: 72/(14+1) = 4.8M bps  
                                                           // 间隔1CLK, 若低先,加0x0400
  SPI1->CNTRL2  = 0x80000000 ;                      
  SPI1->DIVIDER = 4 ;                                      // MEM: 144/(4+1) = 28.8M bps  
                                                                        
  PDMA_GCR->GCRCSR |= 0x0700 ;                             // 接通 DMA012 时钟
  PDMA_GCR->PDSSR0  = 0xFFFF01FF ;                         // SPI1->TX:DMA0, SPI->RX:DMA1
                                      
  PDMA0->DAR  = (uint32_t)(&SPI1->TX[0]) ; 
  PDMA1->SAR  = (uint32_t)(&SPI1->RX[0]) ;                 // 源地址: SPI 接收 BUFF                                                                                                                                                                   
                                                      //MEM 地址,传输字节BCR,传输前再指定       
  PDMA0->ISR  = ~0 ;                                       // 清 0 中断标志
  PDMA2->ISR  = ~0 ;   PDMA3->ISR  = ~0 ;   
  PDMA4->ISR  = ~0 ;   PDMA5->ISR  = ~0 ;   
  PDMA0->IER  =  0 ;   PDMA1->IER  =  3 ;                  // PDMA 开中断
  PDMA2->IER  =  0 ;   PDMA3->IER  =  0 ;                                      
  PDMA4->IER  =  0 ;   PDMA5->IER  =  0 ;                                        
  NVIC_SetPriority(PDMA_IRQn, 3) ;                         // 中断优先级为3
  NVIC->ISER[0] = 1 << PDMA_IRQn ;                         // 打开中断向量
}                                                                   
