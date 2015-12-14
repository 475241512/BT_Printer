//=======================================================================================
// Copyright (C) 2014 Nuvoton Technology(SH) Corp. All rights reserved.
// Ver: 1.00.00    2014/4/28
//=======================================================================================
#include  "NUC123Series.h"
#include  "NUC123_CLK_GPIO_IP_Init.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h"

uint8_t String[64] = "11о�Ƶ��ӿƼ�2345ABCDEFGHIJKLMNOPQRSTUVWXYZ" ;
//=======================================================================================
int32_t main(void)
{
  
  uint32_t temp32, Cnt ;
#ifdef PRINTER_USB_VCOM       
  uint8_t Str[16] = "\n  USB���⴮��OK" ;
  uint8_t *pString = String, *ptr0 ;
  uint16_t *ptr1 ;
#else 
  uint32_t  P_Temper = HeatTimBase + (UpdataPtTime()) ;    
#endif
  
  CLK_Init() ;                                             // �õ�������,���ﶼҪʹ��ʱ��
  ADCInit() ;                                              // 50us��, ADC ֵ��׼ȷ
  Timer1Init() ;                                           // 1MHz����,Delayus()�������ʱ
  Timer2Init() ;                                           // 8Mhz����,DelayCLK()�������ʱ
  USB_Init() ;
  PWM_Init() ;                                             // ״ָ̬ʾ LED �� PWM
  UART0Init(115200) ;                                  
  SPI_Init();                                              // ���ֿ��д��ӡ������ SPI
  GPIO_Init() ;                                            // ��������GPIO�ڹ���
                                                           // UART �ɷ���Ϣ��,LED��ָʾ��
  Tx0FillFiFo("\nUART0 OK !",11) ;
//  while((UART0->FSR & 0x10000000) == 0) ;                  // �����һ���ַ����ͽ���
//  while((UART0->FSR & 0x00400000) == 0) ;                  // �� Tx_FiFo ��    

//  ControlWord = MOTOR_AUTO_SLOW ;                          // �Զ�����ʹ��
//  PRT_Flag |= PRINT_UART_DATA ;                            // �����,�ϵ�ʹ�ӡString
  HeatTimBase  = 500 ;                                     // ��������ʱ�� 500us
  pDataIn      = (uint8_t*)LineData16 ;
  pDataOut     = (uint8_t*)LineData16 ;
  StepIncrease = 0 ;                                       // ��PC���յ��Ľ�ֽ����
  PRT_Flag     = 0 ;
  PA->DOUT    &= ~0x1000 ;                                 // ��ʼ�� Motor �� Heater ����
  AfterOneStep = EveryStep ;
  pStepM_TimLimit = &StepM_TimBase[15] ;                   // ��ֽ����, ����ʱ�ı�
  //// ��ʼ������ ///////////////////////////////////////////////////////////////////////                                  
  
  while(1){
//    PB->DOUT ^= 0x1010 ;                                   // Only for test
    
    if(PD->PIN & 0x800) SYS->GPA_MFP |= 0x4000 ;           // ��ֽ LED��˸
    else                SYS->GPA_MFP &= ~0x4000 ;          // ��ֽ����

#ifdef PRINTER_USB_VCOM     
    if(PRT_Flag & PRINT_UART_DATA){                        // ���ַ��ȴ���ӡ��ֽ��
      ptr0    = (uint8_t*)LineData16 ;                  
      ClearRAM(ptr0, 768) ;                                // �����ӡ������
      pString = String ;                                   // ����ӡ���ַ���
      temp32  = 0 ;  
      do{      
        if(*pString == '\n') break ;                       // ������
        else{ 
          PlaceFont16(ptr0, pString) ;                     // ����16X16������
          if(*pString < 0x80){
            ++pString ; 
            ptr0 += 16 ;                                   // ASC ��, ������16Byte
          }
          else{
            pString += 2 ; 
            ptr0 += 32 ;                                   // ����, ������32Byte  
          }            
        }
      }while(++temp32 < 48) ;                              // һ��384����,���48��ASC16���ַ�
      
      Cnt = Print_Line16((uint8_t*)LineData16,             // ��ӡ�ַ�
                             String[0]-0x31,               // �߶ȵ�һ���ַ���
                             String[1]-0x31) ;             // ��ȵڶ����ַ���
      Cnt = Print_Line16((uint8_t*)LineData16,              
                             String[0]-0x30,              
                             String[1]-0x30) ;             // 16�е���           
      ptr1 = LineData16 ;                              
      ClearRAM((uint8_t*)ptr1, 1536) ;                     // �����ӡ������
      pString = String ;                                   // ����ӡ���ַ���
      temp32  = 0 ;                                 
      do{      
        if(*pString == '\n') break ;                       // ������
        else{ 
          PlaceFont24(ptr1, pString) ;                     // ���� 24 X 24 ������
          if(*pString < 0x80){
            ++pString ; 
            ptr1 += 24 ;                                   // ASC ��, 24 HByte
          }
          else{                                     
            pString += 2 ;              
            ptr1    += 48 ;                                // ����,   48 HByte, 
          }       
        }
      }while(++temp32 < 32) ;                              // һ��384����,���32��ASC24�ַ�

      while(RemainderStep+3 > Cnt) ;                      // ���߹�ȥ�м��, 3��
      Cnt = Print_Line24(LineData16, String[0]-0x30, String[1]-0x30); // ��ӡ 24 X 24 �ַ��� 
      while(PD->DOUT & 0x0400) ;                           // ����ֽ����    
      ClearRAM(String, 64) ;                               // ����ַ������� 
      PRT_Flag  &= ~PRINT_UART_DATA ;                      // ���Խ������ַ���ӡ��      
    }
    if(++Str[1] > 0x39) Str[1] = 0x30 ;                    // ���ּ�1, ��10��0
    if(Vcom_TxSize == 0) Send_to_VCOM(Str,16) ;            // ���ַ������� VCOM    
    Delayus(100000) ;                                      // ��ʱ 0.1s     

//// USB Printer ========================================================================
#else    
    if(StepIncrease){                                      // �յ���ӡ��ֽ����
      temp32 = StepIncrease ;   
      if(RemainderStep) RemainderStep += temp32 ;          // �ӳ���ֽ����
      else{
        while(PD->DOUT & 0x0400) ;                         // ����ֽ��ȫ����     
        PaperRollStart(temp32, 0) ;                        // ����������ǰ��ֽ      
      }
      StepIncrease -= temp32 ;
      P_Temper = HeatTimBase + (UpdataPtTime()) ;  
    }
    if(RemainderStep){                                     // ����ֽ
      if(((pDataOut+48) <= pDataIn)                        // ����һ�����ݴ���ӡ
        || ((pDataOut > pDataIn)&&((pDataIn+PRINT_BUFFER_BYTE-pDataOut)>=48))){
        Cnt = RemainderStep ;                              
        while(Cnt == RemainderStep) ;
        Print_Dot(P_Temper) ;
      }
      else if(RemainderStep > 1000) RemainderStep = 1000 ; // �޴�ӡ����,��ֽ����̫��
    }
   else{                                                  // �޽�ֽ����,������ӡ���ݽ϶�
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

// �൱�� VCOM ���ͽ����ж�, ���Կ�ʼ������һ��������////////////////////////////////////
void USB_Tx_OVER(void)
{
  Vcom_TxSize = 0 ;                              //USB���ݷ������, ����������㹩��Ѱ��
}

// �൱�� VCOM �����ж�, �յ�����, �ȴ����� /////////////////////////////////////////////
 void USB_Rx_Data(void)
{
  uint8_t Cnt = 0 ;
  
  uint32_t RxNum = USBD->EP[3].MXPLD ;                     //�յ����ֽڸ���, ���64��
  uint8_t *ptr   = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ;  //USB���ջ���     
  uint8_t *p     = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[2].BUFSEG) ;  //USB���ͻ���                                          

  while(RxNum--){ 
    String[Cnt++] = *ptr ;                                 // �յ������ݷ���String
    *p++ = *ptr++ - 0x20 ;                                 // ��ĸ���д���ͳ�ȥ
  }
  USBD->EP[2].MXPLD = USBD->EP[3].MXPLD ;
  PRT_Flag |= PRINT_UART_DATA ; 
}                                                //�˳����������������������ݽ���

void GPAB_IRQHandler(void)
{
  uint8_t const str[] = "11о�Ƶ��ӿƼ��Ϻ����޹�˾" ; 
  uint32_t Cnt ;
  
  if((PA->PIN & 0x8000) == 0){
    for(Cnt=0; Cnt<28; ++Cnt) String[Cnt] = str[Cnt] ; 
    PRT_Flag |= PRINT_UART_DATA ;
  }
  PA->ISRC = ~0 ; 
}
