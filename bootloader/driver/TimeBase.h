#ifndef _TIMEBASE_H_
#define _TIMEBASE_H_


void Delay(unsigned short delay);
void TimeBase_Init(void);
void StartDelay(unsigned short nTime);
void TimerStart(void);
void TimerStop(void);
int TimerState(void);
void StopDelay(void);
unsigned char DelayIsEnd(void);
void TIM2_UpdateISRHandler(void);

#endif