#ifndef  __NUC123_HW_H
#define  __NUC123_HW_H

void  UnlockReg(void) ;
void  CLK_Init(void) ;
void  ADCInit(void) ;
void  Timer1Init(void) ;
void  Timer2Init(void) ;
void  PWM_Init(void) ;
void  UART0Init(uint32_t BAUD) ;
void  SPI_Init(void) ;
void  GPIO_Init(void) ;

void  Delayus(uint32_t Tim) ;                              // 1 represent 1us
void  Delay(uint32_t Tim) ;                                // 1 represent 1/8 us

//// 把 UART0 Tx FiFo 填满,不等发送结束就返回 ///////////////////////////////////////////
uint8_t* Tx0FillFiFo(uint8_t*pStr, uint32_t Cnt) ;

#endif  //__NUC123_HW_H
