#ifndef  __PRINTER_H
#define  __PRINTER_H

#ifdef     PRINTER_GLOBAL
  #define  EXT_PRT
#else
  #define  EXT_PRT  extern
#endif

#define  ADC4_CH_THER              4                       // 打印头温度通道
#define  ADC5_CH_VH                5                       // VH 通道ADC
#define  PTR_LATCH(x)           (*(uint32_t*)(GPIO_PIN_DATA_BASE+(0x40*0)+(13<<2)) = (x)) 

EXT_PRT  uint32_t PRT_Flag  ;
#define  PRINT_RECEIVING           1
#define  PRINT_UART_DATA           0x10

#define PRINT_BUFFER_BYTE          4096                     // 24*48 = 1152Byte
EXT_PRT uint16_t __attribute__ ((aligned(4))) LineData16[PRINT_BUFFER_BYTE>>1]; 
EXT_PRT uint8_t  *volatile pDataIn, *volatile pDataOut ;
EXT_PRT uint32_t volatile StepIncrease ;
#define DATA_BUF_END           ((uint8_t*)(&LineData16[(PRINT_BUFFER_BYTE>>1)-1]+1))

EXT_PRT  uint32_t HeatTimBase ;                            // 基础加热时间
EXT_PRT  uint32_t ControlWord ;
#define  MOTOR_AUTO_SLOW           1                       // 自动减速

EXT_PRT  uint32_t PtrStatus ;
#define  PtrS_NO_ERR               0x08
#define  PtrS_SELECTED             0x10
#define  PtrS_NO_PAPER             0x20
 
extern   uint32_t ASC16_Base ;                             // 16行 X 8 列 ASC 码字库
                                          // HZK16 和 HZK24S 放在 SPI FLASH W25Q16 中 
void Delayus(uint32_t Tim) ;
void DelayCLK(uint32_t Tim) ;                              // 1 mean 1/8 us 
void Timeing2_Tick(uint32_t Time) ;                        // 启动Timer2的中断定时
uint32_t UpdataPtTime(void) ;
// 打印 16 X 16 一行汉字或数字 //////////////////////////////////////////////////////////
//input: pData, 字符数据首地址
//       High,  字高(High+1),  range 0~7, 高位会清零
//       Width, 0一倍宽,1两倍宽,其它值4倍宽.
// 返回: 打印完后, 剩余走纸行数
uint32_t Print_Line16(uint8_t*pData, uint8_t High,uint8_t Width); 
uint32_t Print_Line24(uint16_t*pData,uint8_t High,uint8_t Width); //打印24X24字符行
void  ClearRAM(uint8_t *pRAM, uint32_t Cnt) ;         // 清除 RAM 区 Cnt 个字节
void  PlaceFont16(uint8_t *pRAM, uint8_t *OneChar);   // 读出一个 16X16 字模,放入*PRAM
void  PlaceFont24(uint16_t *pRAM, uint8_t *OneChar);  // 读出一个 24X24 字模,放入*pRAM

void     Print_Dot(uint32_t P_Tempera) ;
void     USB_Next_Package(void) ;

// 启动走纸////////////////// 走纸时PD10输出1, 走纸结束时PD10输出0 //////////////////////
// Input  Step,      Stepmotor 步数,即卷纸长度 
//        Diraction, 卷纸方向, 零向前, 非零向后                   
void PaperRollStart(uint16_t Step, uint16_t Diraction) ;        

//== Variable to control step motor =====================================================
extern  uint32_t volatile  RemainderStep      ;            // 最后一步时,值可能为0          
extern  uint32_t           StepM_Backward     ;            // Zero mean forward
extern  uint32_t  const    *pStepM_Out        ;            // pointer of output value
extern  uint16_t  const    (*const StepM_TimBase)[2] ;    
extern  uint16_t  const    (*pStepM_Tim)[2]   ;            // Current speed pointer
extern  uint16_t  const    (*pStepM_TimLimit)[2]    ;      // pointer of Speed limit 
extern  void (*AfterOneStep)(void*pT1_IntPara) ;           // Execute after every Step
extern  void *pT1_IntPara ;
void    EveryStep(void*pParameter) ;
#endif  // __PRINTER_H
