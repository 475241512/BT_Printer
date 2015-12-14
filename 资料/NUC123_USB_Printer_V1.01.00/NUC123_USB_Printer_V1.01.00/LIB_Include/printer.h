#ifndef  __PRINTER_H
#define  __PRINTER_H

#ifdef     PRINTER_GLOBAL
  #define  EXT_PRT
#else
  #define  EXT_PRT  extern
#endif

#define  ADC4_CH_THER              4                       // ��ӡͷ�¶�ͨ��
#define  ADC5_CH_VH                5                       // VH ͨ��ADC
#define  PTR_LATCH(x)           (*(uint32_t*)(GPIO_PIN_DATA_BASE+(0x40*0)+(13<<2)) = (x)) 

EXT_PRT  uint32_t PRT_Flag  ;
#define  PRINT_RECEIVING           1
#define  PRINT_UART_DATA           0x10

#define PRINT_BUFFER_BYTE          4096                     // 24*48 = 1152Byte
EXT_PRT uint16_t __attribute__ ((aligned(4))) LineData16[PRINT_BUFFER_BYTE>>1]; 
EXT_PRT uint8_t  *volatile pDataIn, *volatile pDataOut ;
EXT_PRT uint32_t volatile StepIncrease ;
#define DATA_BUF_END           ((uint8_t*)(&LineData16[(PRINT_BUFFER_BYTE>>1)-1]+1))

EXT_PRT  uint32_t HeatTimBase ;                            // ��������ʱ��
EXT_PRT  uint32_t ControlWord ;
#define  MOTOR_AUTO_SLOW           1                       // �Զ�����

EXT_PRT  uint32_t PtrStatus ;
#define  PtrS_NO_ERR               0x08
#define  PtrS_SELECTED             0x10
#define  PtrS_NO_PAPER             0x20
 
extern   uint32_t ASC16_Base ;                             // 16�� X 8 �� ASC ���ֿ�
                                          // HZK16 �� HZK24S ���� SPI FLASH W25Q16 �� 
void Delayus(uint32_t Tim) ;
void DelayCLK(uint32_t Tim) ;                              // 1 mean 1/8 us 
void Timeing2_Tick(uint32_t Time) ;                        // ����Timer2���ж϶�ʱ
uint32_t UpdataPtTime(void) ;
// ��ӡ 16 X 16 һ�к��ֻ����� //////////////////////////////////////////////////////////
//input: pData, �ַ������׵�ַ
//       High,  �ָ�(High+1),  range 0~7, ��λ������
//       Width, 0һ����,1������,����ֵ4����.
// ����: ��ӡ���, ʣ����ֽ����
uint32_t Print_Line16(uint8_t*pData, uint8_t High,uint8_t Width); 
uint32_t Print_Line24(uint16_t*pData,uint8_t High,uint8_t Width); //��ӡ24X24�ַ���
void  ClearRAM(uint8_t *pRAM, uint32_t Cnt) ;         // ��� RAM �� Cnt ���ֽ�
void  PlaceFont16(uint8_t *pRAM, uint8_t *OneChar);   // ����һ�� 16X16 ��ģ,����*PRAM
void  PlaceFont24(uint16_t *pRAM, uint8_t *OneChar);  // ����һ�� 24X24 ��ģ,����*pRAM

void     Print_Dot(uint32_t P_Tempera) ;
void     USB_Next_Package(void) ;

// ������ֽ////////////////// ��ֽʱPD10���1, ��ֽ����ʱPD10���0 //////////////////////
// Input  Step,      Stepmotor ����,����ֽ���� 
//        Diraction, ��ֽ����, ����ǰ, �������                   
void PaperRollStart(uint16_t Step, uint16_t Diraction) ;        

//== Variable to control step motor =====================================================
extern  uint32_t volatile  RemainderStep      ;            // ���һ��ʱ,ֵ����Ϊ0          
extern  uint32_t           StepM_Backward     ;            // Zero mean forward
extern  uint32_t  const    *pStepM_Out        ;            // pointer of output value
extern  uint16_t  const    (*const StepM_TimBase)[2] ;    
extern  uint16_t  const    (*pStepM_Tim)[2]   ;            // Current speed pointer
extern  uint16_t  const    (*pStepM_TimLimit)[2]    ;      // pointer of Speed limit 
extern  void (*AfterOneStep)(void*pT1_IntPara) ;           // Execute after every Step
extern  void *pT1_IntPara ;
void    EveryStep(void*pParameter) ;
#endif  // __PRINTER_H
