//=======================================================================================
// Copyright (C) 2014 Nuvoton Technology(SH) Corp. All rights reserved.
// Ver: 1.00.00    2014/4/28
//=======================================================================================
#define   PRINTER_GLOBAL
#include  "NUC123Series.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h" 

///////////////////////////////////////////////////////////// ���RAM�� /////////////////
void ClearRAM(uint8_t *pRAM, uint32_t Cnt)
{                                                          // 16X16һ��16*384/8= 768 Byte
  while(Cnt){                                              // 24X24һ��(384/12)*2*24=1536
    *pRAM++ = 0 ;                                          // PC 24�е� 48*24 = 1152 Byte
    --Cnt ;
  }
}
///////////////////////////////////////////////////////////// ���� Timer2(8MHz)��ʱ /////
void Timeing2_Tick(uint32_t Time)                          // ��ʱ����ʱ, ֹͣ����  
{
  if(Time < 9) Time = 9 ;                                  // ֵ��Ҫ̫��
  TIMER2->TCSR  &= ~0x20000000 ;                           
  TIMER2->TISR   = ~0 ;                                    // ���жϱ�־
  TIMER2->TCMPR  = TIMER2->TDR + (Time<<3) ;               // ��ʱʱ��
  TIMER2->TCSR  |= 0x20000000 ;                            // ���ж�
}
///////////////////////////////////////////////////////////// Timer2��ʱʱ�䵽 //////////
void TMR2_IRQHandler(void)                                             
{                                                          // Stop other task if have
  PC->DOUT     &= ~0x3F00 ;                                // ֹͣ����                                
  TIMER2->TCSR &= ~0x20000000 ;                            // ���ж�
  TIMER2->TISR  = ~0 ;                                     // ���жϱ�־
}
///////////////////////////////////////////////////////////// ���ز�����1�ĸ���  ////////  
static uint8_t CalculateOne(uint32_t Data)                       
{
  uint32_t Cnt = 8, temp = 0 ; 
  uint8_t const TabOneNum[16] = {0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4} ; 
  do{
    temp  += TabOneNum[Data&0xF] ;
    Data >>= 4 ;
  }while(--Cnt) ;
  return temp ;
}
///////////////////////////////////////////////////////////// �������� //////////////////
void StepMotor_Slow(void)
{ 
  if(ControlWord & MOTOR_AUTO_SLOW){                       // ����������Զ�����
    pStepM_TimLimit = pStepM_Tim ;                         // ��ǰ�ٶ�,������
    if(pStepM_TimLimit > StepM_TimBase) --pStepM_TimLimit ;
    if(pStepM_TimLimit > StepM_TimBase) --pStepM_TimLimit ;  
  }
}

// ���ݴ�ӡͷ�¶ȵõ�����ʱ��, 30 Dot �����Ҳ���һ�� ////////////////////////////////////
uint32_t UpdataPtTime(void)                                  
{
  uint32_t temp32;
  uint16_t temp16 ;
    
  ADC->CHER  = (1<<ADC4_CH_THER) ;
  ADC->ADCR |= ADC_ADCR_ADST_Msk ;                         //���� ADC ����      
  while(ADC->ADCR & ADC_ADCR_ADST_Msk) ;                   
  temp16 = (uint16_t)ADC->ADDR[ADC4_CH_THER] ; 
                                    
  if(temp16 > 1000)    {temp32 = 1245; pStepM_TimLimit=&StepM_TimBase[7];}  //<-20,=-25
  else if(temp16 > 982){temp32 = 1125; pStepM_TimLimit=&StepM_TimBase[9];}  //<-10,=-15
  else if(temp16 > 953){temp32 = 1005; pStepM_TimLimit=&StepM_TimBase[13];} // < 0,=-5
  else if(temp16 > 911){temp32 = 885; }                             // С��10��,�� 5����
  else if(temp16 > 854){temp32 = 765; }                             // С��20��,��15����
  else if(temp16 > 781){temp32 = 645; }                             // С��30��,��25����
  else if(temp16 > 696){temp32 = 525; }                             // С��40��,��35����
  else if(temp16 > 603){temp32 = 455; }                             // С��50��,��45����
  else if(temp16 > 510){temp32 = 380; }                             // С��60��,��55����
  else if(temp16 > 423){temp32 = 306; }                             // С��70��,��65����
  else                 {temp32 = 0; }                               // ��70��, ֹͣ��ӡ  
  temp32 = (temp32 * 169) >> 8 ;                                    // 8.5V ��ѹ����
  return temp32 ;
}

// �����ܶ���������ʱ��  ////////////////////////////////////////////////////////////////
static uint32_t GetHeatTim(uint8_t NumofOne, uint32_t PT)  // ���64����ͬʱ����, ����3A
{    
  uint16_t const TabQc[8] = {236,246,256,269,282,294,307,323} ;      // Qc = Tab/256
  uint32_t temp = NumofOne >> 4 ;
  
  if(temp > 7) temp = 7 ;
  return (PT*TabQc[temp])>>8 ;                  
//  return (PT*TabQc[temp]*((*pStepM_Tim)[1]))>>16 ;    // ����ʱ������ֽ�����й�ϵ��???               
} //������ֽ�������,ע����ֽ������pStepTim���������,ֵ��Ч,���Բ�Ҫ�ڲ���ֽʱ���ô˺���

static void HeatPaper_1(uint32_t OneNum, uint32_t Tim)
{
  if((PD->PIN & 0x800) == 0) return ;                      // ��ֽ������
  if(OneNum < 64){
    PC->DOUT |= 0x3F00 ;                                   // ���ȿ�ʼ   
    if(Tim > (*pStepM_Tim)[0]){                            // ����ʱ�䲻�ܳ�������ʱ��
      Tim  = (*pStepM_Tim)[0] ;                  
      StepMotor_Slow() ;
    }
    Timeing2_Tick(Tim) ;                                   // ������ʱ�ͷ���, 
  }
  else if(OneNum < 128){
    if(Tim > ((*pStepM_Tim)[0])>>1){
      Tim  = ((*pStepM_Tim)[0]) >> 1 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3800 ;                                   // ���������    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;                             // �ȴ���һ����Ƚ���
    PC->DOUT |= 0x0700 ;                                  
    Timeing2_Tick(Tim) ;    
  }
}
// �ͳ�һ�е����� 48Byte, ����ʼ���� ////////////////////////////////////////////////////
void Print_Dot(uint32_t P_Tempera)
{
  uint32_t temp32, Colum, OneCnt ;                              
  
  PC->DOUT       &= ~0x3F00 ;                              // ֹͣ����     
  SPI0->CNTRL   = 0x00201004 ;                             // FiFo,32λ,�������·����ո���  
  OneCnt = 0 ;
  Colum  = 12 ;                                            // 12 * 32 = 384
  do{                                                      // 4���ֽ���ϳ�һ�� 32λ��
    temp32   = *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    OneCnt += CalculateOne(temp32) ;                    
    while(SPI0->STATUS & 0x08000000) ;                     // FiFo ����, ����
    SPI0->TX[0] = temp32 ;
  }while(--Colum) ;    
  while(SPI0->CNTRL & 1) ;                                 // ��384�з��ͽ���
  PTR_LATCH(0) ;                                         
    temp32 = GetHeatTim(OneCnt, P_Tempera) ;               // ����ʱ��    
    USB_Next_Package() ;
  PTR_LATCH(1) ;         
  HeatPaper_1(OneCnt, temp32) ;                              // ����, ���������, �еȴ�
}

// ��ֽ�ٶ�,�ͼ���ʱ���, ΢��ֵ, Timer1���ó�΢�붨ʱ ==================================
uint16_t const StepM_Time[26][2] = {                     
 {4875,497},{3013,382},{2327,343},{1953,317}, {1712,297},{1541,281},{1412,268},{1310,257},
 {1227,247},{1158,238},{1099,230},{1048,223}, {1004,217},{964, 211},{929, 205},{898, 200},
 {869, 195},{843, 191},{819, 186},{797, 182}, {777, 178},{758, 175},{741, 171},{725,168},
 {709, 165},{694, 161} };                                // ����ʱ������ֽ�����й�ϵ��??? 
uint16_t  const (*const StepM_TimBase)[2] = StepM_Time ; // �ٶȱ���ַָ��

// ������ֽ///////// ��ֽʱPD10���1, ��ֽ����ʱPD10���0, �� Step motor �ϵ� ///////////
// Input  Step,      Stepmotor ����,����ֽ���� 
//        Diraction, ��ֽ����, ����ǰ,�������                 
void PaperRollStart(uint16_t Step, uint16_t Diraction)
{
  PD->DOUT       |=  0x0400 ;                              // ����ӵ�,��ֽ������ϵ�
  RemainderStep   =  Step ;                                // ���һ��ʱRemainderStep=0
  StepM_Backward  =  Diraction ;                           // ��ֽ����,����ǰ,�������
  pStepM_Tim      =  StepM_TimBase ;                       // ��ǰ�ٶ�
  TIMER1->TISR    = ~0 ;                                   // ���жϱ�־
  TIMER1->TCMPR   =  TIMER1->TDR + 5000 ;                  // �ȼӵ�5000us,����ֽ
  TIMER1->TCSR   |=  0x20000000 ;                          // ���ʹ���ж�,  
}                                                          // Timer1ÿ�ж�һ��Motor��һ�� 

////=====================================================================================
//   Micro printer ָ����� , �ⲿ�ִ����� USB �ж���ִ�� 
////=====================================================================================
static uint8_t DataBuffer[16];                             // ���� Printer ָ��
static uint8_t *pDataBuffer = DataBuffer ;
/////////////////////////////////////////////////////////////  USB_Printer //////////////
void  PTR_Data_Receive(void) 
{
  uint8_t *ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ; // EP[3]���ջ���
  int32_t Cnt = USBD->EP[3].MXPLD ;                                     // �յ������ݸ���
  static uint32_t Dot_Data = 0 ;                           // ��������״̬
  static int32_t  LineRec, CntByte ;                               
  
//   do{
//     while(UART0->FSR & 0x00800000 ) ; 
//     UART0->THR = *ptr++ ;   
//   }while(--Cnt);
//   ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ; // EP[3]���ջ���
//   Cnt = USBD->EP[3].MXPLD ;                                     // �յ������ݸ���
  
  if(Dot_Data){                                            // ���ڷ���������
    do{ *pDataIn = *ptr++;                                 // �����������     
      if(++pDataIn >= DATA_BUF_END) pDataIn = (uint8_t*)LineData16 ;
      --Cnt;  --CntByte;
    }while(Cnt && CntByte) ;             
    if(CntByte <= 0){    Dot_Data = 0 ;                    // �������ݽ���  
      pDataBuffer = DataBuffer ;                           // ���´�������
    }      
  }
  
  if(Cnt){                                                 // USB ���ջ��������ݴ�����
    do{
      /////////////////////////////////////////////////////// ������GS��ESCͷ ///////////
      if(pDataBuffer == DataBuffer){             
        if((*ptr == 0x1B) || (*ptr == 0x1D)) *pDataBuffer++ = *ptr ;                                 
      }
      /////////////////////////////////////////////////////// printer����: GS ///////////
      else if(DataBuffer[0] == 0x1D){                      
        if(pDataBuffer <= &DataBuffer[1])*pDataBuffer++ = *ptr ;  // ����ڶ����ֽ�                            
        else{
          if(DataBuffer[1] == 0x76){                       // Dot ��������ʼ
            if(pDataBuffer < &DataBuffer[8])*pDataBuffer++ = *ptr; // ����1D76����8���ֽ�
            else{
              LineRec = DataBuffer[7];  LineRec <<= 8;  LineRec += DataBuffer[6];
              CntByte = DataBuffer[5];  CntByte <<= 8;  CntByte += DataBuffer[4];
              CntByte = CntByte * LineRec ;
                      
              while(Cnt){       
                *pDataIn = *ptr++ ;                      // ���汾���еĵ�������
                 if(++pDataIn >= DATA_BUF_END) pDataIn = (uint8_t*)LineData16 ;
                 --Cnt;  --CntByte ;  
              } ;                          
              Dot_Data = 1 ;  
              break ;                  
            }
          }
          else pDataBuffer = DataBuffer ;                  // 1D ��� 76, ����������ͷ              
        }
      }
      /////////////////////////////////////////////////////// printer����: ESC //////////
      else if(DataBuffer[0] == 0x1B){
        if(*ptr == 0x4A){                                  // ��ֽ��ӡ����        
          StepIncrease += *(ptr+1) ;
        }
        else if(*ptr == 0x64){                             
           StepIncrease += *(ptr+1)*24 ;
        }
        else if(*ptr == 0x40){                             // Print ��ʼ��ָ�� 
//            pDataIn  = (uint8_t*)LineData16 ;
//            do *pDataIn++ = 0 ; while(pDataIn < DATA_BUF_END) ; 
//            pDataIn  = (uint8_t*)LineData16 ;
//            pDataOut = (uint8_t*)LineData16 ;
//            StepIncrease = 0 ;
//            if(RemainderStep > 100) RemainderStep = 100 ;    // ��ֽһ�ξ�ͣ
        }
        pDataBuffer = DataBuffer  ;                        // ����������ͷ                             
      }
      ///////////////////////////////////////////////////////////////////////////////////
      ++ptr ;
    }while(--Cnt) ;                            
  }  
  // ���Ƿ�Ҫ������һ������ /////////////////////////////////////////////////////////////
   USB_Next_Package() ;
}
//// �������ռ����64Byte�ͽ�����һ�� ///////////////////////////////////////////////////
void USB_Next_Package(void)
{
  if(PRT_Flag & PRINT_RECEIVING) return ;                  // ����������
  if(pDataIn < pDataOut){
    if((pDataOut - pDataIn) > 64)  EP3_Rx_Ready() ;        //�˵�3 Rx׼��������һ������ 
  }
  else if((pDataOut + PRINT_BUFFER_BYTE - pDataIn) > 64) EP3_Rx_Ready();
}
// Execute every step of motor //////////////////////////////////////////////////////////
void EveryStep(void*pParameter)                           
{
//  PA->DOUT ^= 0x8000 ;                                     // Only for test
}

///== ͨ�� USB ���⴮�ڴ�ӡ�Ĵ��� =======================================================
#ifdef PRINTER_USB_VCOM     
///////////////////////////////////////////////////////////// ���� //////////////////////
static void HeatPaper(uint32_t OneNum, uint32_t Tim)
{
  if((PD->PIN & 0x800) == 0) return ;                      // ��ֽ������
  if(OneNum < 64){
    PC->DOUT |= 0x3F00 ;                                   // ���ȿ�ʼ   
    if(Tim > ((*pStepM_Tim)[0]<<1)){                       // ����ʱ�䲻�ܳ�������ʱ��
      Tim  = ((*pStepM_Tim)[0]<<1) - 50 ;                  
      StepMotor_Slow() ;
    }
    Timeing2_Tick(Tim) ;                                   // ������ʱ�ͷ���, 
  }
  else if(OneNum < 128){
    if(Tim > (*pStepM_Tim)[0]){
      Tim  = (*pStepM_Tim)[0] - 50 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3800 ;                                   // ���������    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;                             // �ȴ���һ����Ƚ���
    PC->DOUT |= 0x0700 ;                                  
    Timeing2_Tick(Tim) ;    
  }
  else{
    if(Tim*3 > ((*pStepM_Tim)[0]<<1)){
      Tim  = ((*pStepM_Tim)[0]*2/3) - 50 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3000 ;                                   // ���������    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;           
    PC->DOUT |= 0x0C00 ;                                 
    Timeing2_Tick(Tim) ; 
    while(PC->DOUT & 0x3F00) ;                             // �ȴ�ǰ������Ƚ���
    PC->DOUT |= 0x0300 ;                               
    Timeing2_Tick(Tim) ;    
  } 
 }
///////////////////////////////////////////////////////////// ���ַ�����һ�� ////////////
static uint32_t StrenthByte(uint16_t Data)            
{
  uint32_t temp32 = 0 ;
  uint32_t Cnt = 16 ;                                   // ����ֵ�ǲ�����16λչ��ɵ�32λ   
  
  do{                                                 
    temp32 <<= 2 ;
    if(Data & 0x8000) temp32 |= 3 ;
    Data   <<= 1 ;
  }while(--Cnt) ;  
  return temp32 ; 
}

// ��ӡ 16 X 16 һ�к��ֻ����� //////////////////////////////////////////////////////////
//input: pData, �ַ������׵�ַ, ÿ16���ֽ���һ���ַ�����ģ. һ�������������ַ�
//       High,  �ָ�(High+1),  range 0~7, ��λ������
//       Width, 0һ����,1������,����ֵ4����.
// ����: ��ӡ���, ʣ����ֽ����
uint32_t Print_Line16(uint8_t*pData, uint8_t High, uint8_t Width)
{
  uint32_t temp32, P_Temperature, CurrentLine ;                                     
  uint32_t Colum, height, OneCnt, Line = 0 ;                               
  uint8_t *pImgData ;
  uint16_t const  (*pStpM_TLimit)[2]   ; 
  
  Width &= 3 ;
  High   = ((High & 7) + 1) << 1 ;                         // 8���ַ��߶�2,4,...16
  temp32 = (High<<4) + 26 ;                                // �м�����23, ��������Ҫ����
  if(RemainderStep){  
    if(RemainderStep < temp32) RemainderStep = temp32 ;    // ��ֽ��������"�������",����
  }
  else{
    while(PD->DOUT & 0x400) ;                              // �����һ������
    PaperRollStart(temp32,0) ;                             // ����������ǰ��ֽ      
  }
  P_Temperature = HeatTimBase + UpdataPtTime() ;           // ��ӡͷ����
  SPI0->CNTRL   = 0x00201004 ;                             // FiFo,32λ,�������·����ո���        
  CurrentLine   = RemainderStep - 2 ;                      // ��ӡ��ʼ��
  pStpM_TLimit  = pStepM_TimLimit ;                        // �ݴ�����ֵ
  
  do{
    pImgData = pData + Line ;
    OneCnt   = 0 ;
    Colum    = 12 ;                                        // 12 * 32 = 384 Dot
    do{
      switch(Width){
      case 0 :                                             // �ֿ�һ��,4Byte���һ��
        temp32   = *pImgData ;                 pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        break ;
      
      case 1 :                                             // �ֿ�����,2Byte�����4Byte
        temp32   = StrenthByte(*pImgData);  pImgData += 16 ;
        temp32 <<= 16 ;
        temp32  |= StrenthByte(*pImgData);  pImgData += 16 ;
      
        break ;
      default :                                            // �ֿ�4��,1Byte����4��
        temp32  = StrenthByte(*pImgData) ;  pImgData += 16 ;
        temp32  = StrenthByte(temp32) ;
      }
      
      OneCnt += CalculateOne(temp32) ;                    
      while(SPI0->STATUS & 0x08000000) ;                   // FiFo ����, ����
      SPI0->TX[0] = temp32 ;
    }while(--Colum);
    while(SPI0->CNTRL & 1) ;                               // ��384���㷢�ͽ���

    while(RemainderStep > CurrentLine) ;                   // ���µ������п�ʼ 
    PTR_LATCH(0) ;                                         // ��������  
      PC->DOUT &= ~0x3F00 ;                                // ֹͣ����     
      pStepM_TimLimit = pStpM_TLimit  ;                    // �ָ�����
      temp32 = GetHeatTim(OneCnt,P_Temperature) ;          // ����ʱ��    
      height = High ;
    PTR_LATCH(1) ;  
    
    while(height){
      height -= 2 ;                                        // ����Dot�м���һ��
      while(RemainderStep > CurrentLine) ;                 // ���µ������п�ʼ
      CurrentLine   = RemainderStep-2  ;
      HeatPaper(OneCnt, temp32);      
    }        
  }while(++Line < 16) ;                                    // ֱ��16�����������
  pStepM_TimLimit =  pStpM_TLimit  ;
  while(RemainderStep > CurrentLine) ;                     // �����һ�е��ӡ����
  return RemainderStep ;                                   // ���ص�ǰʣ�����ֽ����
}

// ��ӡһ�� 24 X 24 �ַ���,����ָ���������ֽ�, �� 24*(384/12)*2 = 1536 Byte /////////////
// ÿ24��Half wordһ����ģ, ��12λЧ,  Width�����ֿ�ӱ�, 
uint32_t Print_Line24(uint16_t*pData, uint8_t High, uint8_t Width)
{
  uint32_t temp32, P_Temperature, CurrentLine ;
  uint32_t Colum, height,OneCnt, Line = 0 ;                              
  uint16_t *pImgData ;
  uint16_t const  (*pStpM_TLimit)[2]   ; 
  
  Width &= 1 ;
  High   = ((High & 7) + 1) << 1 ;                         // 2,4,6....16
  temp32 = (High*24) + 26 ;      
  if(RemainderStep){  
    if(RemainderStep < temp32) RemainderStep = temp32 ;    // �ӳ���ֽ����
  }
  else{
    while(PD->DOUT & 0x0400) ;                             // ����ֽ��ȫ����     
    PaperRollStart(temp32,0) ;                             // ������ǰ��ֽ      
  }
  P_Temperature = HeatTimBase + (UpdataPtTime()) ;         // ��ӡͷ����,
  SPI0->CNTRL   = 0x00201004 + (24<<3) ;                   // FiFo,24λ,�������·����ո���        
  CurrentLine   = RemainderStep - 2 ;                      // ��ʼ��
  pStpM_TLimit  = pStepM_TimLimit ;
  
  do{
    pImgData = pData + Line ;
    OneCnt   = 0 ;
    Colum    = 16 ;                                        // 16 * 24 = 384
    do{
      if(Width){                                           // ��ȼӱ�
        temp32   = StrenthByte(*pImgData) ; pImgData += 24 ; 
      }
      else{
        temp32   = *pImgData ;  pImgData += 24 ;
        temp32 <<= 12 ;  temp32 |= ((uint32_t)(*pImgData)) ; pImgData += 24 ;
      }
      OneCnt += CalculateOne(temp32) ;                    
      while(SPI0->STATUS & 0x08000000) ;                   // FiFo ����, ����
      SPI0->TX[0] = temp32 ;
    }while(--Colum) ;
    while(SPI0->CNTRL & 1) ;                               // ��384�з��ͽ���

    while(RemainderStep > CurrentLine) ;                   // �ȿ�ʼ��  
    PTR_LATCH(0) ;                                         
      PC->DOUT &= ~0x3F00 ;                                // ֹͣ����     
      pStepM_TimLimit =  pStpM_TLimit  ;             
      temp32 = GetHeatTim(OneCnt, P_Temperature) ;         // ����ʱ��    
      height = High ;
    PTR_LATCH(1) ;  
    
    while(height){
      height -= 2 ;      
      while(RemainderStep > CurrentLine) ;                 // ���µ�������
      CurrentLine  = RemainderStep-2  ;
      HeatPaper(OneCnt, temp32) ;      
    }        
    
  }while(++Line < 24) ;                                    // 24 �������������
  pStepM_TimLimit =  pStpM_TLimit  ;
  while(RemainderStep > CurrentLine) ;                     // �����һ�е��ӡ����
  return RemainderStep ;                                  
}

// ���� PDMA �󷵻�: ��SPI Memory����һ��16X16��ģ, ���� RAM_Address ////////////////////
// PD9_CS ��� 1 ʱ����Ż����,��PDMA�ж���, 
// 16X16��ģ�ֳ�����8X16 ASC16��ĸ�ʽ, �ֳ���16�ֽ�, ����16�ֽ�
static void ReadFont16_16(uint32_t SPIData_Address, uint8_t *RAM_Address)
{
  PD->DOUT   &= ~0x200 ;                                   // PD9_/CS = 0 
  SPI1->CNTRL = 0x00201004 + (8<<3);                       //FiFo,8λ,�������·����ո���   
  SPI1->TX[0] =  0x03 ;                                    // SPI MEM ����
  SPI1->TX[0] =  SPIData_Address >> 16 ;         
  SPI1->TX[0] =  SPIData_Address >> 8 ;         
  SPI1->TX[0] =  SPIData_Address ;        
  SPI1->DMA   =  0x00000004 ;                              // ��λ PDMA

  PDMA0->CSR  = 0x00000089 +(1 << 19) ;                    // 8λ,Դ��,Ŀ�Ĺ̶�,MEM->IP                                                                       //��8λ�� 0x00080000
  PDMA1->CSR  = 0x00000025 +(1 << 19) ;                    // 8λ,Դ�̶�,Ŀ�ļ�,IP->MEM  
  PDMA0->CSR |=  2 ;                                       // ��λ PDMA0
  PDMA1->CSR |=  2 ;                                       // ��λ PDMA1
  PDMA1->DAR  = (uint32_t)RAM_Address ;                    // ��Ŷ������ݵ� RAM ��ַ
  PDMA0->SAR  = (uint32_t)RAM_Address ;     
  PDMA0->BCR  = 32 ;                                   
  PDMA1->BCR  = 32 ;                                       // �����ֽڸ���
  PDMA1->CSR |= 0x00800000 ;                            
  PDMA0->CSR |= 0x00800000 ;                               // DAM ��������, �Զ���0
  while(SPI1->CNTRL & 1) ;                                 // �ȵ�һ����"��ַ"����
  SPI1->FIFO_CTL= 1 ;                                      // ��ս��� FiFo 
  SPI1->DMA     = 0x00000003 ;                             // ��ͨ DMA �Ĵ����ź�,�Զ���0
}
// �� SPI MEM �ж����� PDMA ���� ////////////////////////////////////////////////////////
void PDMA_IRQHandler(void)                                 // SPI_PDMA01 �������
{
  PD->DOUT   |=  0x200 ;                                   // /CS of SPI Mem = 1 
  PDMA1->ISR  = ~0 ;                                       // �� 0 PDMA �жϱ�־
}
// ���� PDMA �󷵻�: ��SPI Memory����һ��24X24��ģ, ���� RAM_Address ////////////////////
// PD9 ��� 1 ʱ����Ż����,��PDMA�ж���.
// 24X24��ģ�ֳ�����12X24 ASC16��ĸ�ʽ, �ֳ���24����, ����24����, ��12λ��Ч
static void ReadFont24_24(uint32_t SPIData_Address, uint16_t *RAM_Address)
{
  uint32_t temp32 = 0x03000000 + SPIData_Address ;         // SPI MEM ����
  
  PD->DOUT   &= ~0x200 ;                                   // PD9_/CS = 0 
  SPI1->CNTRL = 0x00201004 + (16<<3);                      //FiFo,16λ,�������·����ո���   
  SPI1->TX[0] =  temp32 >> 16 ;                            
  SPI1->TX[0] =  temp32 ;         
  SPI1->DMA   =  0x00000004 ;                              // ��λ PDMA

  PDMA0->CSR |=  2 ;                                       // ��λ PDMA0
  PDMA1->CSR |=  2 ;                                       // ��λ PDMA1
  PDMA0->CSR  = 0x00000089 +(2 << 19) ;                    // 16λ,Դ��,Ŀ�Ĺ̶�,MEM->IP                                                                       //��8λ�� 0x00080000
  PDMA1->CSR  = 0x00000025 +(2 << 19) ;                    // 16λ,Դ�̶�,Ŀ�ļ�,IP->MEM  
  PDMA1->DAR  = (uint32_t)RAM_Address ;                    // ��Ŷ������ݵ� RAM ��ַ
  PDMA0->SAR  = (uint32_t)RAM_Address ;     
  PDMA0->BCR  = 96 ;                                   
  PDMA1->BCR  = 96 ;                                       // �����ֽڸ���
  PDMA1->CSR |= 0x00800000 ;                            
  PDMA0->CSR |= 0x00800000 ;                               // DAM ��������, �Զ���0
  while(SPI1->CNTRL & 1) ;                                 // �ȵ�һ����"��ַ"����
  SPI1->FIFO_CTL= 1 ;                                      // ��ս��� FiFo 
  SPI1->DMA     = 0x00000003 ;                             // ��ͨ DMA �Ĵ����ź�,�Զ���0
}

// ���ַ� OneChar ��16 Byte��ģ���ݶ��뵽 pRAM �� //////////////////////////////////////
void PlaceFont16(uint8_t *pRAM, uint8_t *OneChar)
{
  uint8_t n=16, *p ; 
  
  if(*OneChar & 0x80){                                     // ����
    ReadFont16_16(((*OneChar-0xA1)*94 + *(OneChar+1)-0xA1)*32, pRAM) ;
    while((PD->DOUT & 0x200) == 0) ;                       // �� DMA �������   
  }
  else{                                                    // ���ֺ�Ӣ����ĸ
    p = (uint8_t*)(&ASC16_Base) + (*OneChar << 4) ;
    do *pRAM++ = *p++ ; while(--n) ;
  }
}

uint16_t const ASC24_Base[6144] = {                        //2*24*128 = 6144
0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,           // NULL
0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x030,0x030,0x070,0x0F0,0x1F0,           // 1
0x330,0x230,0x030,0x030,0x030,0x030,0x030,0x030,
0x030,0x030,0x030,0x030,0x030,0x030,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x1FC,0x30C,0x706,0x606,           // 2
0x606,0x006,0x00E,0x00C,0x01C,0x038,0x070,0x060,
0x0E0,0x1C0,0x380,0x300,0x7FE,0x7FE,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x3F8,0x31C,0x60C,0x60C,           // 3
0x00C,0x00C,0x018,0x070,0x07C,0x00C,0x006,0x006,
0x006,0x606,0x60E,0x31C,0x3F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x00C,0x01C,0x01C,0x03C,0x06C,           // 4
0x06C,0x0CC,0x0CC,0x18C,0x38C,0x30C,0x60C,0x7FF,
0x7FF,0x00C,0x00C,0x00C,0x00C,0x00C,0x000,0x000,
0x000,0x000,0x000,0x1FC,0x1FC,0x380,0x300,0x300,           // 5
0x300,0x3F0,0x7F8,0x71C,0x60E,0x006,0x006,0x006,
0x006,0x606,0x60C,0x31C,0x3F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x0F8,0x1FC,0x38C,0x306,0x206,           // 6
0x600,0x600,0x670,0x7FC,0x70C,0x70E,0x606,0x606,
0x606,0x606,0x30E,0x38C,0x1F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x7FE,0x7FE,0x006,0x00C,0x018,           // 7
0x018,0x030,0x030,0x060,0x060,0x060,0x0C0,0x0C0,
0x0C0,0x0C0,0x180,0x180,0x180,0x180,0x000,0x000,  
0x000,0x000,0x000,0x0F0,0x1F8,0x39C,0x30C,0x30C,           // 8
0x30C,0x30C,0x198,0x0F0,0x1F8,0x30C,0x606,0x606,
0x606,0x606,0x606,0x30C,0x3FC,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x1F8,0x38C,0x70C,0x606,           // 9
0x606,0x606,0x606,0x70E,0x30E,0x3FE,0x0E6,0x006,
0x006,0x60C,0x60C,0x31C,0x3F8,0x1E0,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x0E0,0x0E0,0x1B0,           // A
0x1B0,0x1B0,0x3B8,0x318,0x318,0x318,0x7FC,0x7FC,
0x60C,0x60C,0xC06,0xC06,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7F0,0x7F8,0x61C,           // B
0x60C,0x60C,0x618,0x7F0,0x7F8,0x61C,0x60C,0x60C,
0x60C,0x61C,0x7F8,0x7F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x0F0,0x1FC,0x38C,           // C
0x30E,0x606,0x600,0x600,0x600,0x600,0x606,0x606,
0x30C,0x39C,0x1F8,0x0F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7F0,0x7F8,0x61C,           // D
0x60C,0x606,0x606,0x606,0x606,0x606,0x606,0x606,
0x60C,0x61C,0x7F8,0x7F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7FC,0x7FC,0x600,           // E
0x600,0x600,0x600,0x7FC,0x7FC,0x600,0x600,0x600,
0x600,0x600,0x7FC,0x7FC,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7FC,0x7FC,0x600,           // F
0x600,0x600,0x600,0x7F8,0x7F8,0x600,0x600,0x600,
0x600,0x600,0x600,0x600,0x000,0x000,0x000,0x000,

0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x10
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x11
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x12
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x13
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x14
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x15
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x16
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x17
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x18
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x19
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1A
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1B
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1C
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1D
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1E
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x1F
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x20 �ո�
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x21
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x22
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x23
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x24
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x24
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x26
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x27
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x28
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x29
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2A
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2B
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2C
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2D
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2E
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x2F
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x000,0x000,0x000,0x0F0,0x1F8,0x39C,0x70E,0x606,           // 0x30
0x606,0x606,0x606,0x606,0x606,0x606,0x606,0x606,
0x606,0x606,0x70E,0x39C,0x1F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x030,0x030,0x070,0x0F0,0x1F0,           // 0x31
0x330,0x230,0x030,0x030,0x030,0x030,0x030,0x030,
0x030,0x030,0x030,0x030,0x030,0x030,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x1FC,0x30C,0x706,0x606,           // 0x32
0x606,0x006,0x00E,0x00C,0x01C,0x038,0x070,0x060,
0x0E0,0x1C0,0x380,0x300,0x7FE,0x7FE,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x3F8,0x31C,0x60C,0x60C,           // 0x33
0x00C,0x00C,0x018,0x070,0x07C,0x00C,0x006,0x006,
0x006,0x606,0x60E,0x31C,0x3F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x00C,0x01C,0x01C,0x03C,0x06C,           // 0x34
0x06C,0x0CC,0x0CC,0x18C,0x38C,0x30C,0x60C,0x7FF,
0x7FF,0x00C,0x00C,0x00C,0x00C,0x00C,0x000,0x000,
0x000,0x000,0x000,0x1FC,0x1FC,0x380,0x300,0x300,           // 0x35
0x300,0x3F0,0x7F8,0x71C,0x60E,0x006,0x006,0x006,
0x006,0x606,0x60C,0x31C,0x3F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x0F8,0x1FC,0x38C,0x306,0x206,           // 0x36
0x600,0x600,0x670,0x7FC,0x70C,0x70E,0x606,0x606,
0x606,0x606,0x30E,0x38C,0x1F8,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x7FE,0x7FE,0x006,0x00C,0x018,           // 0x37
0x018,0x030,0x030,0x060,0x060,0x060,0x0C0,0x0C0,
0x0C0,0x0C0,0x180,0x180,0x180,0x180,0x000,0x000,  
0x000,0x000,0x000,0x0F0,0x1F8,0x39C,0x30C,0x30C,           // 0x38
0x30C,0x30C,0x198,0x0F0,0x1F8,0x30C,0x606,0x606,
0x606,0x606,0x606,0x30C,0x3FC,0x0F0,0x000,0x000,
0x000,0x000,0x000,0x0F0,0x1F8,0x38C,0x70C,0x606,           // 0x39
0x606,0x606,0x606,0x70E,0x30E,0x3FE,0x0E6,0x006,
0x006,0x60C,0x60C,0x31C,0x3F8,0x1E0,0x000,0x000,
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3A
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3B
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3C
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3D
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3E
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x3F
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x40
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,  

0x000,0x000,0x000,0x000,0x000,0x0E0,0x0E0,0x1B0,           // 0x41 A
0x1B0,0x1B0,0x3B8,0x318,0x318,0x318,0x7FC,0x7FC,
0x60C,0x60C,0xC06,0xC06,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7F0,0x7F8,0x61C,           // 0x42 B
0x60C,0x60C,0x618,0x7F0,0x7F8,0x61C,0x60C,0x60C,
0x60C,0x61C,0x7F8,0x7F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x0F0,0x1FC,0x38C,           // 0x43 C
0x30E,0x606,0x600,0x600,0x600,0x600,0x606,0x606,
0x30C,0x39C,0x1F8,0x0F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7F0,0x7F8,0x61C,           // 0x44 D
0x60C,0x606,0x606,0x606,0x606,0x606,0x606,0x606,
0x60C,0x61C,0x7F8,0x7F0,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7FC,0x7FC,0x600,           // 0x45 E
0x600,0x600,0x600,0x7FC,0x7FC,0x600,0x600,0x600,
0x600,0x600,0x7FC,0x7FC,0x000,0x000,0x000,0x000,
0x000,0x000,0x000,0x000,0x000,0x7FC,0x7FC,0x600,           // 0x46 F
0x600,0x600,0x600,0x7F8,0x7F8,0x600,0x600,0x600,
0x600,0x600,0x600,0x600,0x000,0x000,0x000,0x000,
}  ;

// ���ַ� OneChar �� 24 Half word ��ģ���ݶ��뵽 pRAM �� ////////////////////////////////
void PlaceFont24(uint16_t *pRAM, uint8_t *OneChar)
{
  uint32_t n = 24 ; 
  uint16_t const *p ;     
  
  if(*OneChar & 0x80){                                     // ����
    ReadFont24_24(0x50000+((*OneChar-0xB0)*94 + *(OneChar+1)-0xA1)*96, pRAM) ;
    while((PD->DOUT & 0x200) == 0) ;                       // �� DMA �������   
  }
  else{                                                    // ���ֺ�Ӣ����ĸ
    p = ASC24_Base + (*OneChar * 24) ;
    do *pRAM++ = *p++ ;  while(--n) ;
  }
}

#endif
