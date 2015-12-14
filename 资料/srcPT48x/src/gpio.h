/****************************************************************************
 *   $Id:: gpio.h 4067 2010-07-30 02:23:08Z usb00423                        $
 *   Project: NXP LPC11xx software example
 *
 *   Description:
 *     This file contains definition and prototype for GPIO.
 *
 ****************************************************************************
 * Software that is described herein is for illustrative purposes only
 * which provides customers with programming information regarding the
 * products. This software is supplied "AS IS" without any warranties.
 * NXP Semiconductors assumes no responsibility or liability for the
 * use of the software, conveys no license or title under any patent,
 * copyright, or mask work right to the product. NXP Semiconductors
 * reserves the right to make changes in the software without
 * notification. NXP Semiconductors also make no representation or
 * warranty that such application will be suitable for the specified
 * use without further testing or modification.
****************************************************************************/
#ifndef __GPIO_H
#define __GPIO_H

#define PORT0		0
#define PORT1		1
#define PORT2		2
#define PORT3		3
#define GPIO_Pin_0                 0    /*!< Pin 0 selected */
#define GPIO_Pin_1                 1    /*!< Pin 1 selected */
#define GPIO_Pin_2                 2    /*!< Pin 2 selected */
#define GPIO_Pin_3                 3    /*!< Pin 3 selected */
#define GPIO_Pin_4                 4    /*!< Pin 4 selected */
#define GPIO_Pin_5                 5    /*!< Pin 5 selected */
#define GPIO_Pin_6                 6    /*!< Pin 6 selected */
#define GPIO_Pin_7                 7    /*!< Pin 7 selected */
#define GPIO_Pin_8                 8    /*!< Pin 8 selected */
#define GPIO_Pin_9                 9    /*!< Pin 9 selected */
#define GPIO_Pin_10                10   /*!< Pin 10 selected */
#define GPIO_Pin_11                11   /*!< Pin 11 selected */
#define Output      1
#define Input       0
#define Pull_down   (1<<3)
#define Pull_up     (1<<4)
#define Hysteresis  (1<<5)


static LPC_GPIO_TypeDef (* const LPC_GPIO[4]) = { LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3 };

void GPIOSetValue( uint32_t portNum, uint32_t bitPosi, uint32_t bitVal );
void GPIOSetDir( uint32_t portNum, uint32_t bitPosi, uint32_t dir );
uint8_t GPIO_ReadInputDataBit(uint32_t portNum,uint32_t bitPosi);

#endif /* end __GPIO_H */
/*****************************************************************************
**                            End Of File
******************************************************************************/
