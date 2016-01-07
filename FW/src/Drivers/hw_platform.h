#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

#define		HW_VER_DEMO_V11		0		//主控使用的是F103VET6（100PIN），支持4个硬件蓝牙模块，对应的原理图是《BTPrinter_DemoBoard_V1.10》
#define		HW_VER_V11			1		//主控使用的是F103RCT6（64PIN）, 只支持2个硬件蓝牙模块，对应的原理图是《BTPrinter_V1.10》
#define		HW_VER_V12			2		//主控使用的是F103C8/BT6（48PIN）, 只支持1个硬件蓝牙模块+LCD，对应的原理图是《BTPrinter_V1.20》

//#define     HW_VER			HW_VER_DEMO_V11
#define     HW_VER			HW_VER_V11
//#define     HW_VER			HW_VER_V12	

#define USE_BT1_MODULE		(1<<0)
#define USE_BT2_MODULE		(1<<1)
#define USE_BT3_MODULE		(1<<2)
#define USE_BT4_MODULE		(1<<3)

//此宏配置，根据具体的硬件实际使用那几个蓝牙模块来决定的
//#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4个蓝牙模块都同时工作
#if(HW_VER == HW_VER_V12)
#define BT_MODULE_CONFIG	(USE_BT1_MODULE)		//1个蓝牙模块都同时工作
#elif(HW_VER == HW_VER_V11)
//#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE)		//2个蓝牙模块都同时工作
#define BT_MODULE_CONFIG	(USE_BT1_MODULE)		//1个蓝牙模块都同时工作
#else
#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4个蓝牙模块都同时工作
#endif

//蓝牙模块类型
#define		USE_WBTDS01		1
#define		USE_BT816		2

//定义使用的蓝牙模块
#define BT_MODULE		USE_BT816

#define SMALL_MEMORY_CFG			//开启小内存模式，开启此宏是为了尽量节省内存，将整个项目的内存空间尽量压缩到20K

#ifdef SMALL_MEMORY_CFG
#define SOFT_TIMER_FUNC_ENABLE			0
#else
#define SOFT_TIMER_FUNC_ENABLE			1
#endif

#include "spi_flash.h"
#include "data_uart.h"
#include "BT816.h"
#include "Esc_p.h"
#include "print_head.h"
#include "PaperDetect.h"
#include  "KeyScan.h"
#include "basic_fun.h"
#ifdef LCD_VER
#include "Lcd.h"
#else
#include "LED.h"
#endif
void hw_platform_init(void);
unsigned int hw_platform_USBcable_Insert_Detect(void);

#endif
