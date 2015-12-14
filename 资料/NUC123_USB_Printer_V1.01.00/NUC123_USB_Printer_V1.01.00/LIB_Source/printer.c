//=======================================================================================
// Copyright (C) 2014 Nuvoton Technology(SH) Corp. All rights reserved.
// Ver: 1.00.00    2014/4/28
//=======================================================================================
#define   PRINTER_GLOBAL
#include  "NUC123Series.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h" 

///////////////////////////////////////////////////////////// 清除RAM区 /////////////////
void ClearRAM(uint8_t *pRAM, uint32_t Cnt)
{                                                          // 16X16一行16*384/8= 768 Byte
  while(Cnt){                                              // 24X24一行(384/12)*2*24=1536
    *pRAM++ = 0 ;                                          // PC 24行点 48*24 = 1152 Byte
    --Cnt ;
  }
}
///////////////////////////////////////////////////////////// 启动 Timer2(8MHz)定时 /////
void Timeing2_Tick(uint32_t Time)                          // 定时结束时, 停止加热  
{
  if(Time < 9) Time = 9 ;                                  // 值不要太短
  TIMER2->TCSR  &= ~0x20000000 ;                           
  TIMER2->TISR   = ~0 ;                                    // 清中断标志
  TIMER2->TCMPR  = TIMER2->TDR + (Time<<3) ;               // 定时时长
  TIMER2->TCSR  |= 0x20000000 ;                            // 开中断
}
///////////////////////////////////////////////////////////// Timer2定时时间到 //////////
void TMR2_IRQHandler(void)                                             
{                                                          // Stop other task if have
  PC->DOUT     &= ~0x3F00 ;                                // 停止加热                                
  TIMER2->TCSR &= ~0x20000000 ;                            // 关中断
  TIMER2->TISR  = ~0 ;                                     // 清中断标志
}
///////////////////////////////////////////////////////////// 返回参数中1的个数  ////////  
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
///////////////////////////////////////////////////////////// 降速两级 //////////////////
void StepMotor_Slow(void)
{ 
  if(ControlWord & MOTOR_AUTO_SLOW){                       // 如果配置了自动降速
    pStepM_TimLimit = pStepM_Tim ;                         // 当前速度,降两级
    if(pStepM_TimLimit > StepM_TimBase) --pStepM_TimLimit ;
    if(pStepM_TimLimit > StepM_TimBase) --pStepM_TimLimit ;  
  }
}

// 根据打印头温度得到加热时间, 30 Dot 行左右测温一次 ////////////////////////////////////
uint32_t UpdataPtTime(void)                                  
{
  uint32_t temp32;
  uint16_t temp16 ;
    
  ADC->CHER  = (1<<ADC4_CH_THER) ;
  ADC->ADCR |= ADC_ADCR_ADST_Msk ;                         //启动 ADC 测温      
  while(ADC->ADCR & ADC_ADCR_ADST_Msk) ;                   
  temp16 = (uint16_t)ADC->ADDR[ADC4_CH_THER] ; 
                                    
  if(temp16 > 1000)    {temp32 = 1245; pStepM_TimLimit=&StepM_TimBase[7];}  //<-20,=-25
  else if(temp16 > 982){temp32 = 1125; pStepM_TimLimit=&StepM_TimBase[9];}  //<-10,=-15
  else if(temp16 > 953){temp32 = 1005; pStepM_TimLimit=&StepM_TimBase[13];} // < 0,=-5
  else if(temp16 > 911){temp32 = 885; }                             // 小于10度,按 5度算
  else if(temp16 > 854){temp32 = 765; }                             // 小于20度,按15度算
  else if(temp16 > 781){temp32 = 645; }                             // 小于30度,按25度算
  else if(temp16 > 696){temp32 = 525; }                             // 小于40度,按35度算
  else if(temp16 > 603){temp32 = 455; }                             // 小于50度,按45度算
  else if(temp16 > 510){temp32 = 380; }                             // 小于60度,按55度算
  else if(temp16 > 423){temp32 = 306; }                             // 小于70度,按65度算
  else                 {temp32 = 0; }                               // 超70度, 停止打印  
  temp32 = (temp32 * 169) >> 8 ;                                    // 8.5V 电压修正
  return temp32 ;
}

// 按点密度修正加热时间  ////////////////////////////////////////////////////////////////
static uint32_t GetHeatTim(uint8_t NumofOne, uint32_t PT)  // 最多64个点同时加热, 电流3A
{    
  uint16_t const TabQc[8] = {236,246,256,269,282,294,307,323} ;      // Qc = Tab/256
  uint32_t temp = NumofOne >> 4 ;
  
  if(temp > 7) temp = 7 ;
  return (PT*TabQc[temp])>>8 ;                  
//  return (PT*TabQc[temp]*((*pStepM_Tim)[1]))>>16 ;    // 加热时间与走纸快慢有关系吗???               
} //若按走纸步数查表,注意走纸结束后pStepTim可能下溢出,值无效,所以不要在不走纸时调用此函数

static void HeatPaper_1(uint32_t OneNum, uint32_t Tim)
{
  if((PD->PIN & 0x800) == 0) return ;                      // 无纸不加热
  if(OneNum < 64){
    PC->DOUT |= 0x3F00 ;                                   // 加热开始   
    if(Tim > (*pStepM_Tim)[0]){                            // 加热时间不能超过步进时间
      Tim  = (*pStepM_Tim)[0] ;                  
      StepMotor_Slow() ;
    }
    Timeing2_Tick(Tim) ;                                   // 启动定时就返回, 
  }
  else if(OneNum < 128){
    if(Tim > ((*pStepM_Tim)[0])>>1){
      Tim  = ((*pStepM_Tim)[0]) >> 1 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3800 ;                                   // 分两组加热    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;                             // 等待第一组加热结束
    PC->DOUT |= 0x0700 ;                                  
    Timeing2_Tick(Tim) ;    
  }
}
// 送出一行点数据 48Byte, 并开始加热 ////////////////////////////////////////////////////
void Print_Dot(uint32_t P_Tempera)
{
  uint32_t temp32, Colum, OneCnt ;                              
  
  PC->DOUT       &= ~0x3F00 ;                              // 停止加热     
  SPI0->CNTRL   = 0x00201004 ;                             // FiFo,32位,正脉冲下发上收高先  
  OneCnt = 0 ;
  Colum  = 12 ;                                            // 12 * 32 = 384
  do{                                                      // 4个字节组合成一个 32位数
    temp32   = *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    temp32 <<= 8 ;  temp32 |= *pDataOut ; 
    if(++pDataOut >= DATA_BUF_END) pDataOut = (uint8_t*)LineData16 ;    
    OneCnt += CalculateOne(temp32) ;                    
    while(SPI0->STATUS & 0x08000000) ;                     // FiFo 满了, 等着
    SPI0->TX[0] = temp32 ;
  }while(--Colum) ;    
  while(SPI0->CNTRL & 1) ;                                 // 等384列发送结束
  PTR_LATCH(0) ;                                         
    temp32 = GetHeatTim(OneCnt, P_Tempera) ;               // 加热时间    
    USB_Next_Package() ;
  PTR_LATCH(1) ;         
  HeatPaper_1(OneCnt, temp32) ;                              // 加热, 若分组加热, 有等待
}

// 走纸速度,和加热时间表, 微秒值, Timer1配置成微秒定时 ==================================
uint16_t const StepM_Time[26][2] = {                     
 {4875,497},{3013,382},{2327,343},{1953,317}, {1712,297},{1541,281},{1412,268},{1310,257},
 {1227,247},{1158,238},{1099,230},{1048,223}, {1004,217},{964, 211},{929, 205},{898, 200},
 {869, 195},{843, 191},{819, 186},{797, 182}, {777, 178},{758, 175},{741, 171},{725,168},
 {709, 165},{694, 161} };                                // 加热时间与走纸快慢有关系吗??? 
uint16_t  const (*const StepM_TimBase)[2] = StepM_Time ; // 速度表格基址指针

// 启动走纸///////// 走纸时PD10输出1, 走纸结束时PD10输出0, 给 Step motor 断电 ///////////
// Input  Step,      Stepmotor 步数,即卷纸长度 
//        Diraction, 卷纸方向, 零向前,非零向后                 
void PaperRollStart(uint16_t Step, uint16_t Diraction)
{
  PD->DOUT       |=  0x0400 ;                              // 电机加电,走纸结束后断电
  RemainderStep   =  Step ;                                // 最后一步时RemainderStep=0
  StepM_Backward  =  Diraction ;                           // 走纸方向,零向前,非零向后
  pStepM_Tim      =  StepM_TimBase ;                       // 当前速度
  TIMER1->TISR    = ~0 ;                                   // 清中断标志
  TIMER1->TCMPR   =  TIMER1->TDR + 5000 ;                  // 先加电5000us,再走纸
  TIMER1->TCSR   |=  0x20000000 ;                          // 最后使能中断,  
}                                                          // Timer1每中断一次Motor走一步 

////=====================================================================================
//   Micro printer 指令解析 , 这部分代码在 USB 中断里执行 
////=====================================================================================
static uint8_t DataBuffer[16];                             // 缓存 Printer 指令
static uint8_t *pDataBuffer = DataBuffer ;
/////////////////////////////////////////////////////////////  USB_Printer //////////////
void  PTR_Data_Receive(void) 
{
  uint8_t *ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ; // EP[3]接收缓存
  int32_t Cnt = USBD->EP[3].MXPLD ;                                     // 收到的数据个数
  static uint32_t Dot_Data = 0 ;                           // 点数据流状态
  static int32_t  LineRec, CntByte ;                               
  
//   do{
//     while(UART0->FSR & 0x00800000 ) ; 
//     UART0->THR = *ptr++ ;   
//   }while(--Cnt);
//   ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[3].BUFSEG) ; // EP[3]接收缓存
//   Cnt = USBD->EP[3].MXPLD ;                                     // 收到的数据个数
  
  if(Dot_Data){                                            // 正在发点阵数据
    do{ *pDataIn = *ptr++;                                 // 缓存点阵数据     
      if(++pDataIn >= DATA_BUF_END) pDataIn = (uint8_t*)LineData16 ;
      --Cnt;  --CntByte;
    }while(Cnt && CntByte) ;             
    if(CntByte <= 0){    Dot_Data = 0 ;                    // 点阵数据结束  
      pDataBuffer = DataBuffer ;                           // 重新处理命令
    }      
  }
  
  if(Cnt){                                                 // USB 接收缓存有数据待处理
    do{
      /////////////////////////////////////////////////////// 找命令GS和ESC头 ///////////
      if(pDataBuffer == DataBuffer){             
        if((*ptr == 0x1B) || (*ptr == 0x1D)) *pDataBuffer++ = *ptr ;                                 
      }
      /////////////////////////////////////////////////////// printer命令: GS ///////////
      else if(DataBuffer[0] == 0x1D){                      
        if(pDataBuffer <= &DataBuffer[1])*pDataBuffer++ = *ptr ;  // 缓存第二个字节                            
        else{
          if(DataBuffer[1] == 0x76){                       // Dot 数据流开始
            if(pDataBuffer < &DataBuffer[8])*pDataBuffer++ = *ptr; // 缓存1D76命令8个字节
            else{
              LineRec = DataBuffer[7];  LineRec <<= 8;  LineRec += DataBuffer[6];
              CntByte = DataBuffer[5];  CntByte <<= 8;  CntByte += DataBuffer[4];
              CntByte = CntByte * LineRec ;
                      
              while(Cnt){       
                *pDataIn = *ptr++ ;                      // 缓存本包中的点阵数据
                 if(++pDataIn >= DATA_BUF_END) pDataIn = (uint8_t*)LineData16 ;
                 --Cnt;  --CntByte ;  
              } ;                          
              Dot_Data = 1 ;  
              break ;                  
            }
          }
          else pDataBuffer = DataBuffer ;                  // 1D 后非 76, 重新找命令头              
        }
      }
      /////////////////////////////////////////////////////// printer命令: ESC //////////
      else if(DataBuffer[0] == 0x1B){
        if(*ptr == 0x4A){                                  // 走纸打印命令        
          StepIncrease += *(ptr+1) ;
        }
        else if(*ptr == 0x64){                             
           StepIncrease += *(ptr+1)*24 ;
        }
        else if(*ptr == 0x40){                             // Print 初始化指令 
//            pDataIn  = (uint8_t*)LineData16 ;
//            do *pDataIn++ = 0 ; while(pDataIn < DATA_BUF_END) ; 
//            pDataIn  = (uint8_t*)LineData16 ;
//            pDataOut = (uint8_t*)LineData16 ;
//            StepIncrease = 0 ;
//            if(RemainderStep > 100) RemainderStep = 100 ;    // 走纸一段就停
        }
        pDataBuffer = DataBuffer  ;                        // 重新找命令头                             
      }
      ///////////////////////////////////////////////////////////////////////////////////
      ++ptr ;
    }while(--Cnt) ;                            
  }  
  // 判是否要接收下一包数据 /////////////////////////////////////////////////////////////
   USB_Next_Package() ;
}
//// 缓冲区空间多于64Byte就接收下一包 ///////////////////////////////////////////////////
void USB_Next_Package(void)
{
  if(PRT_Flag & PRINT_RECEIVING) return ;                  // 接收已启动
  if(pDataIn < pDataOut){
    if((pDataOut - pDataIn) > 64)  EP3_Rx_Ready() ;        //端点3 Rx准备接收下一包数据 
  }
  else if((pDataOut + PRINT_BUFFER_BYTE - pDataIn) > 64) EP3_Rx_Ready();
}
// Execute every step of motor //////////////////////////////////////////////////////////
void EveryStep(void*pParameter)                           
{
//  PA->DOUT ^= 0x8000 ;                                     // Only for test
}

///== 通过 USB 虚拟串口打印的代码 =======================================================
#ifdef PRINTER_USB_VCOM     
///////////////////////////////////////////////////////////// 加热 //////////////////////
static void HeatPaper(uint32_t OneNum, uint32_t Tim)
{
  if((PD->PIN & 0x800) == 0) return ;                      // 无纸不加热
  if(OneNum < 64){
    PC->DOUT |= 0x3F00 ;                                   // 加热开始   
    if(Tim > ((*pStepM_Tim)[0]<<1)){                       // 加热时间不能超过步进时间
      Tim  = ((*pStepM_Tim)[0]<<1) - 50 ;                  
      StepMotor_Slow() ;
    }
    Timeing2_Tick(Tim) ;                                   // 启动定时就返回, 
  }
  else if(OneNum < 128){
    if(Tim > (*pStepM_Tim)[0]){
      Tim  = (*pStepM_Tim)[0] - 50 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3800 ;                                   // 分两组加热    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;                             // 等待第一组加热结束
    PC->DOUT |= 0x0700 ;                                  
    Timeing2_Tick(Tim) ;    
  }
  else{
    if(Tim*3 > ((*pStepM_Tim)[0]<<1)){
      Tim  = ((*pStepM_Tim)[0]*2/3) - 50 ;
      StepMotor_Slow() ;
    }               
    PC->DOUT |= 0x3000 ;                                   // 分三组加热    
    Timeing2_Tick(Tim) ;   
    while(PC->DOUT & 0x3F00) ;           
    PC->DOUT |= 0x0C00 ;                                 
    Timeing2_Tick(Tim) ; 
    while(PC->DOUT & 0x3F00) ;                             // 等待前两组加热结束
    PC->DOUT |= 0x0300 ;                               
    Timeing2_Tick(Tim) ;    
  } 
 }
///////////////////////////////////////////////////////////// 把字符拉宽一倍 ////////////
static uint32_t StrenthByte(uint16_t Data)            
{
  uint32_t temp32 = 0 ;
  uint32_t Cnt = 16 ;                                   // 返回值是参数低16位展宽成的32位   
  
  do{                                                 
    temp32 <<= 2 ;
    if(Data & 0x8000) temp32 |= 3 ;
    Data   <<= 1 ;
  }while(--Cnt) ;  
  return temp32 ; 
}

// 打印 16 X 16 一行汉字或数字 //////////////////////////////////////////////////////////
//input: pData, 字符数据首地址, 每16个字节是一个字符的字模. 一个汉字有两个字符
//       High,  字高(High+1),  range 0~7, 高位会清零
//       Width, 0一倍宽,1两倍宽,其它值4倍宽.
// 返回: 打印完后, 剩余走纸行数
uint32_t Print_Line16(uint8_t*pData, uint8_t High, uint8_t Width)
{
  uint32_t temp32, P_Temperature, CurrentLine ;                                     
  uint32_t Colum, height, OneCnt, Line = 0 ;                               
  uint8_t *pImgData ;
  uint16_t const  (*pStpM_TLimit)[2]   ; 
  
  Width &= 3 ;
  High   = ((High & 7) + 1) << 1 ;                         // 8种字符高度2,4,...16
  temp32 = (High<<4) + 26 ;                                // 行间距最多23, 否则这里要增加
  if(RemainderStep){  
    if(RemainderStep < temp32) RemainderStep = temp32 ;    // 走纸长度少于"点的行数",增加
  }
  else{
    while(PD->DOUT & 0x400) ;                              // 等最后一步结束
    PaperRollStart(temp32,0) ;                             // 重新启动向前走纸      
  }
  P_Temperature = HeatTimBase + UpdataPtTime() ;           // 打印头测温
  SPI0->CNTRL   = 0x00201004 ;                             // FiFo,32位,正脉冲下发上收高先        
  CurrentLine   = RemainderStep - 2 ;                      // 打印开始行
  pStpM_TLimit  = pStepM_TimLimit ;                        // 暂存限速值
  
  do{
    pImgData = pData + Line ;
    OneCnt   = 0 ;
    Colum    = 12 ;                                        // 12 * 32 = 384 Dot
    do{
      switch(Width){
      case 0 :                                             // 字宽一倍,4Byte组合一起
        temp32   = *pImgData ;                 pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        temp32 <<= 8 ;  temp32 |= *pImgData ;  pImgData += 16 ;
        break ;
      
      case 1 :                                             // 字宽两倍,2Byte拉宽成4Byte
        temp32   = StrenthByte(*pImgData);  pImgData += 16 ;
        temp32 <<= 16 ;
        temp32  |= StrenthByte(*pImgData);  pImgData += 16 ;
      
        break ;
      default :                                            // 字宽4倍,1Byte拉宽4倍
        temp32  = StrenthByte(*pImgData) ;  pImgData += 16 ;
        temp32  = StrenthByte(temp32) ;
      }
      
      OneCnt += CalculateOne(temp32) ;                    
      while(SPI0->STATUS & 0x08000000) ;                   // FiFo 满了, 等着
      SPI0->TX[0] = temp32 ;
    }while(--Colum);
    while(SPI0->CNTRL & 1) ;                               // 等384个点发送结束

    while(RemainderStep > CurrentLine) ;                   // 等新的数据行开始 
    PTR_LATCH(0) ;                                         // 数据锁存  
      PC->DOUT &= ~0x3F00 ;                                // 停止加热     
      pStepM_TimLimit = pStpM_TLimit  ;                    // 恢复限速
      temp32 = GetHeatTim(OneCnt,P_Temperature) ;          // 加热时间    
      height = High ;
    PTR_LATCH(1) ;  
    
    while(height){
      height -= 2 ;                                        // 两个Dot行加热一次
      while(RemainderStep > CurrentLine) ;                 // 等新的数据行开始
      CurrentLine   = RemainderStep-2  ;
      HeatPaper(OneCnt, temp32);      
    }        
  }while(++Line < 16) ;                                    // 直到16行数据输出完
  pStepM_TimLimit =  pStpM_TLimit  ;
  while(RemainderStep > CurrentLine) ;                     // 等最后一行点打印结束
  return RemainderStep ;                                   // 返回当前剩余的走纸行数
}

// 打印一个 24 X 24 字符行,参数指向数据首字节, 共 24*(384/12)*2 = 1536 Byte /////////////
// 每24个Half word一个字模, 低12位效,  Width非零字宽加倍, 
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
    if(RemainderStep < temp32) RemainderStep = temp32 ;    // 加长走纸长度
  }
  else{
    while(PD->DOUT & 0x0400) ;                             // 等走纸完全结束     
    PaperRollStart(temp32,0) ;                             // 启动向前走纸      
  }
  P_Temperature = HeatTimBase + (UpdataPtTime()) ;         // 打印头测温,
  SPI0->CNTRL   = 0x00201004 + (24<<3) ;                   // FiFo,24位,正脉冲下发上收高先        
  CurrentLine   = RemainderStep - 2 ;                      // 开始行
  pStpM_TLimit  = pStepM_TimLimit ;
  
  do{
    pImgData = pData + Line ;
    OneCnt   = 0 ;
    Colum    = 16 ;                                        // 16 * 24 = 384
    do{
      if(Width){                                           // 宽度加倍
        temp32   = StrenthByte(*pImgData) ; pImgData += 24 ; 
      }
      else{
        temp32   = *pImgData ;  pImgData += 24 ;
        temp32 <<= 12 ;  temp32 |= ((uint32_t)(*pImgData)) ; pImgData += 24 ;
      }
      OneCnt += CalculateOne(temp32) ;                    
      while(SPI0->STATUS & 0x08000000) ;                   // FiFo 满了, 等着
      SPI0->TX[0] = temp32 ;
    }while(--Colum) ;
    while(SPI0->CNTRL & 1) ;                               // 等384列发送结束

    while(RemainderStep > CurrentLine) ;                   // 等开始行  
    PTR_LATCH(0) ;                                         
      PC->DOUT &= ~0x3F00 ;                                // 停止加热     
      pStepM_TimLimit =  pStpM_TLimit  ;             
      temp32 = GetHeatTim(OneCnt, P_Temperature) ;         // 加热时间    
      height = High ;
    PTR_LATCH(1) ;  
    
    while(height){
      height -= 2 ;      
      while(RemainderStep > CurrentLine) ;                 // 等新的数据行
      CurrentLine  = RemainderStep-2  ;
      HeatPaper(OneCnt, temp32) ;      
    }        
    
  }while(++Line < 24) ;                                    // 24 行数据输出结束
  pStepM_TimLimit =  pStpM_TLimit  ;
  while(RemainderStep > CurrentLine) ;                     // 等最后一行点打印结束
  return RemainderStep ;                                  
}

// 启动 PDMA 后返回: 从SPI Memory读出一个16X16字模, 放入 RAM_Address ////////////////////
// PD9_CS 输出 1 时传输才会结束,在PDMA中断里, 
// 16X16字模分成两个8X16 ASC16码的格式, 分成左16字节, 和右16字节
static void ReadFont16_16(uint32_t SPIData_Address, uint8_t *RAM_Address)
{
  PD->DOUT   &= ~0x200 ;                                   // PD9_/CS = 0 
  SPI1->CNTRL = 0x00201004 + (8<<3);                       //FiFo,8位,正脉冲下发上收高先   
  SPI1->TX[0] =  0x03 ;                                    // SPI MEM 读出
  SPI1->TX[0] =  SPIData_Address >> 16 ;         
  SPI1->TX[0] =  SPIData_Address >> 8 ;         
  SPI1->TX[0] =  SPIData_Address ;        
  SPI1->DMA   =  0x00000004 ;                              // 复位 PDMA

  PDMA0->CSR  = 0x00000089 +(1 << 19) ;                    // 8位,源加,目的固定,MEM->IP                                                                       //传8位加 0x00080000
  PDMA1->CSR  = 0x00000025 +(1 << 19) ;                    // 8位,源固定,目的加,IP->MEM  
  PDMA0->CSR |=  2 ;                                       // 复位 PDMA0
  PDMA1->CSR |=  2 ;                                       // 复位 PDMA1
  PDMA1->DAR  = (uint32_t)RAM_Address ;                    // 存放读出数据的 RAM 地址
  PDMA0->SAR  = (uint32_t)RAM_Address ;     
  PDMA0->BCR  = 32 ;                                   
  PDMA1->BCR  = 32 ;                                       // 读出字节个数
  PDMA1->CSR |= 0x00800000 ;                            
  PDMA0->CSR |= 0x00800000 ;                               // DAM 触发传输, 自动清0
  while(SPI1->CNTRL & 1) ;                                 // 等第一个字"地址"发完
  SPI1->FIFO_CTL= 1 ;                                      // 清空接收 FiFo 
  SPI1->DMA     = 0x00000003 ;                             // 接通 DMA 的触发信号,自动清0
}
// 从 SPI MEM 中读数据 PDMA 结束 ////////////////////////////////////////////////////////
void PDMA_IRQHandler(void)                                 // SPI_PDMA01 传输结束
{
  PD->DOUT   |=  0x200 ;                                   // /CS of SPI Mem = 1 
  PDMA1->ISR  = ~0 ;                                       // 清 0 PDMA 中断标志
}
// 启动 PDMA 后返回: 从SPI Memory读出一个24X24字模, 放入 RAM_Address ////////////////////
// PD9 输出 1 时传输才会结束,在PDMA中断里.
// 24X24字模分成两个12X24 ASC16码的格式, 分成左24半字, 和右24半字, 低12位有效
static void ReadFont24_24(uint32_t SPIData_Address, uint16_t *RAM_Address)
{
  uint32_t temp32 = 0x03000000 + SPIData_Address ;         // SPI MEM 读出
  
  PD->DOUT   &= ~0x200 ;                                   // PD9_/CS = 0 
  SPI1->CNTRL = 0x00201004 + (16<<3);                      //FiFo,16位,正脉冲下发上收高先   
  SPI1->TX[0] =  temp32 >> 16 ;                            
  SPI1->TX[0] =  temp32 ;         
  SPI1->DMA   =  0x00000004 ;                              // 复位 PDMA

  PDMA0->CSR |=  2 ;                                       // 复位 PDMA0
  PDMA1->CSR |=  2 ;                                       // 复位 PDMA1
  PDMA0->CSR  = 0x00000089 +(2 << 19) ;                    // 16位,源加,目的固定,MEM->IP                                                                       //传8位加 0x00080000
  PDMA1->CSR  = 0x00000025 +(2 << 19) ;                    // 16位,源固定,目的加,IP->MEM  
  PDMA1->DAR  = (uint32_t)RAM_Address ;                    // 存放读出数据的 RAM 地址
  PDMA0->SAR  = (uint32_t)RAM_Address ;     
  PDMA0->BCR  = 96 ;                                   
  PDMA1->BCR  = 96 ;                                       // 读出字节个数
  PDMA1->CSR |= 0x00800000 ;                            
  PDMA0->CSR |= 0x00800000 ;                               // DAM 触发传输, 自动清0
  while(SPI1->CNTRL & 1) ;                                 // 等第一个字"地址"发完
  SPI1->FIFO_CTL= 1 ;                                      // 清空接收 FiFo 
  SPI1->DMA     = 0x00000003 ;                             // 接通 DMA 的触发信号,自动清0
}

// 把字符 OneChar 的16 Byte字模数据读入到 pRAM 处 //////////////////////////////////////
void PlaceFont16(uint8_t *pRAM, uint8_t *OneChar)
{
  uint8_t n=16, *p ; 
  
  if(*OneChar & 0x80){                                     // 汉字
    ReadFont16_16(((*OneChar-0xA1)*94 + *(OneChar+1)-0xA1)*32, pRAM) ;
    while((PD->DOUT & 0x200) == 0) ;                       // 等 DMA 传输结束   
  }
  else{                                                    // 数字和英文字母
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
0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,0x0000,   //0x20 空格
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

// 把字符 OneChar 的 24 Half word 字模数据读入到 pRAM 处 ////////////////////////////////
void PlaceFont24(uint16_t *pRAM, uint8_t *OneChar)
{
  uint32_t n = 24 ; 
  uint16_t const *p ;     
  
  if(*OneChar & 0x80){                                     // 汉字
    ReadFont24_24(0x50000+((*OneChar-0xB0)*94 + *(OneChar+1)-0xA1)*96, pRAM) ;
    while((PD->DOUT & 0x200) == 0) ;                       // 等 DMA 传输结束   
  }
  else{                                                    // 数字和英文字母
    p = ASC24_Base + (*OneChar * 24) ;
    do *pRAM++ = *p++ ;  while(--n) ;
  }
}

#endif
