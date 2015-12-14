//=======================================================================================
// Copyright (C) 2014 Nuvoton Technology(SH) Corp. All rights reserved.
// Ver: 1.00.00    2014/4/28
//=======================================================================================
#include  "NUC123Series.h"
#include  "NUC123_CLK_GPIO_IP_Init.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h"

uint8_t String[64] = "11芯唐电子科技2345ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
//=======================================================================================
int32_t main(void)
{
  
  uint32_t temp32, Cnt ;
#ifdef PRINTER_USB_VCOM       
  uint8_t Str[16] = "\n  USB虚拟串口OK" ;
  uint8_t *pString = String, *ptr0 ;
  uint16_t *ptr1 ;
#else 
  uint32_t  P_Temper = HeatTimBase + (UpdataPtTime()) ;    
#endif
  
  CLK_Init() ;                                             // 用到的外设,这里都要使能时钟
  ADCInit() ;                                              // 50us后, ADC 值才准确
  Timer1Init() ;                                           // 1MHz计数,Delayus()用这个延时
  Timer2Init() ;                                           // 8Mhz计数,DelayCLK()用这个延时
  USB_Init() ;
  PWM_Init() ;                                             // 状态指示 LED 用 PWM
  UART0Init(115200) ;                                  
  SPI_Init();                                              // 读字库和写打印数据用 SPI
  GPIO_Init() ;                                            // 集中配置GPIO口功能
                                                           // UART 可发信息了,LED可指示了
  Tx0FillFiFo("\nUART0 OK !",11) ;
//  while((UART0->FSR & 0x10000000) == 0) ;                  // 等最后一个字符发送结束
//  while((UART0->FSR & 0x00400000) == 0) ;                  // 等 Tx_FiFo 空    

//  ControlWord = MOTOR_AUTO_SLOW ;                          // 自动减速使能
//  PRT_Flag |= PRINT_UART_DATA ;                            // 有这句,上电就打印String
  HeatTimBase  = 500 ;                                     // 基础加热时间 500us
  pDataIn      = (uint8_t*)LineData16 ;
  pDataOut     = (uint8_t*)LineData16 ;
  StepIncrease = 0 ;                                       // 从PC机收到的进纸步数
  PRT_Flag     = 0 ;
  PA->DOUT    &= ~0x1000 ;                                 // 开始给 Motor 和 Heater 供电
  AfterOneStep = EveryStep ;
  pStepM_TimLimit = &StepM_TimBase[15] ;                   // 走纸限速, 可随时改变
  //// 初始化结束 ///////////////////////////////////////////////////////////////////////                                  
  
  while(1){
//    PB->DOUT ^= 0x1010 ;                                   // Only for test
    
    if(PD->PIN & 0x800) SYS->GPA_MFP |= 0x4000 ;           // 有纸 LED闪烁
    else                SYS->GPA_MFP &= ~0x4000 ;          // 无纸不闪

#ifdef PRINTER_USB_VCOM     
    if(PRT_Flag & PRINT_UART_DATA){                        // 有字符等待打印到纸上
      ptr0    = (uint8_t*)LineData16 ;                  
      ClearRAM(ptr0, 768) ;                                // 清除打印缓冲区
      pString = String ;                                   // 待打印的字符串
      temp32  = 0 ;  
      do{      
        if(*pString == '\n') break ;                       // 结束符
        else{ 
          PlaceFont16(ptr0, pString) ;                     // 读出16X16点数据
          if(*pString < 0x80){
            ++pString ; 
            ptr0 += 16 ;                                   // ASC 码, 读出了16Byte
          }
          else{
            pString += 2 ; 
            ptr0 += 32 ;                                   // 汉字, 读出了32Byte  
          }            
        }
      }while(++temp32 < 48) ;                              // 一行384个点,最多48个ASC16个字符
      
      Cnt = Print_Line16((uint8_t*)LineData16,             // 打印字符
                             String[0]-0x31,               // 高度第一个字符定
                             String[1]-0x31) ;             // 宽度第二个字符定
      Cnt = Print_Line16((uint8_t*)LineData16,              
                             String[0]-0x30,              
                             String[1]-0x30) ;             // 16行点阵           
      ptr1 = LineData16 ;                              
      ClearRAM((uint8_t*)ptr1, 1536) ;                     // 清除打印缓冲区
      pString = String ;                                   // 待打印的字符串
      temp32  = 0 ;                                 
      do{      
        if(*pString == '\n') break ;                       // 结束符
        else{ 
          PlaceFont24(ptr1, pString) ;                     // 读出 24 X 24 点数据
          if(*pString < 0x80){
            ++pString ; 
            ptr1 += 24 ;                                   // ASC 码, 24 HByte
          }
          else{                                     
            pString += 2 ;              
            ptr1    += 48 ;                                // 汉字,   48 HByte, 
          }       
        }
      }while(++temp32 < 32) ;                              // 一行384个点,最多32个ASC24字符

      while(RemainderStep+3 > Cnt) ;                      // 等走过去行间距, 3行
      Cnt = Print_Line24(LineData16, String[0]-0x30, String[1]-0x30); // 打印 24 X 24 字符行 
      while(PD->DOUT & 0x0400) ;                           // 等走纸结束    
      ClearRAM(String, 64) ;                               // 清除字符缓冲区 
      PRT_Flag  &= ~PRINT_UART_DATA ;                      // 可以接受新字符打印了      
    }
    if(++Str[1] > 0x39) Str[1] = 0x30 ;                    // 数字加1, 到10变0
    if(Vcom_TxSize == 0) Send_to_VCOM(Str,16) ;            // 把字符串发到 VCOM    
    Delayus(100000) ;                                      // 延时 0.1s     

//// USB Printer ========================================================================
#else    
    if(StepIncrease){                                      // 收到打印进纸命令
      temp32 = StepIncrease ;   
      if(RemainderStep) RemainderStep += temp32 ;          // 加长走纸长度
      else{
        while(PD->DOUT & 0x0400) ;                         // 等走纸完全结束     
        PaperRollStart(temp32, 0) ;                        // 重新启动向前走纸      
      }
      StepIncrease -= temp32 ;
      P_Temper = HeatTimBase + (UpdataPtTime()) ;  
    }
    if(RemainderStep){                                     // 已走纸
      if(((pDataOut+48) <= pDataIn)                        // 至少一行数据待打印
        || ((pDataOut > pDataIn)&&((pDataIn+PRINT_BUFFER_BYTE-pDataOut)>=48))){
        Cnt = RemainderStep ;                              
        while(Cnt == RemainderStep) ;
        Print_Dot(P_Temper) ;
      }
      else if(RemainderStep > 1000) RemainderStep = 1000 ; // 无打印内容,走纸不能太长
    }
   else{                                                  // 无进纸命令,但待打印数据较多
      if(pDataOut <= pDataIn){
        if((pDataIn - pDataOut) > (48*23)) 
            StepIncrease += 25 ;                  
      }
      else if((pDataIn + PRINT_BUFFER_BYTE - pDataOut) > (48*23)) 
          StepIncrease += 25 ;      
   }
#endif    
  }
}

// 相当于 VCOM 发送结束中断, 可以开始发送下一包数据了////////////////////////////////////
void USB_Tx_OVER(void)
{
  Vcom_TxSize = 0 ;                              //USB数据发送完毕, 这个变量清零供查寻用
}

// 相当于 VCOM 接收中断, 收到数据, 等待处理 /////////////////////////////////////////////
 void USB_Rx_Data(void)
{
  uint8_t Cnt = 0 ;
  
  uint32_t RxNum = USBD->EP[3].MXPLD ;                     //收到的字节个数, 最多64个
  uint8_t *ptr   = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ;  //USB接收缓存     
  uint8_t *p     = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[2].BUFSEG) ;  //USB发送缓存                                          

  while(RxNum--){ 
    String[Cnt++] = *ptr ;                                 // 收到的数据放入String
    *p++ = *ptr++ - 0x20 ;                                 // 字母变大写发送出去
  }
  USBD->EP[2].MXPLD = USBD->EP[3].MXPLD ;
  PRT_Flag |= PRINT_UART_DATA ; 
}                                                //退出这个函数后会重新启动数据接收

void GPAB_IRQHandler(void)
{
  uint8_t const str[] = "11芯唐电子科技上海有限公司" ; 
  uint32_t Cnt ;
  
  if((PA->PIN & 0x8000) == 0){
    for(Cnt=0; Cnt<28; ++Cnt) String[Cnt] = str[Cnt] ; 
    PRT_Flag |= PRINT_UART_DATA ;
  }
  PA->ISRC = ~0 ; 
}
