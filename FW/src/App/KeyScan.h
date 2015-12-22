#ifndef _KEYSCAN_H
#define _KEYSCAN_H

#define KEY_FEED()  ( GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))		//PA.1

void KeyScanProc(void);
void KeyScanInit(void);
void box_ctrl(int ms);

#endif

