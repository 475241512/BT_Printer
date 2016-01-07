#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

#define		HW_VER_DEMO_V11		0		//����ʹ�õ���F103VET6��100PIN����֧��4��Ӳ������ģ�飬��Ӧ��ԭ��ͼ�ǡ�BTPrinter_DemoBoard_V1.10��
#define		HW_VER_V11			1		//����ʹ�õ���F103RCT6��64PIN��, ֻ֧��2��Ӳ������ģ�飬��Ӧ��ԭ��ͼ�ǡ�BTPrinter_V1.10��
#define		HW_VER_V12			2		//����ʹ�õ���F103C8/BT6��48PIN��, ֻ֧��1��Ӳ������ģ��+LCD����Ӧ��ԭ��ͼ�ǡ�BTPrinter_V1.20��

//#define     HW_VER			HW_VER_DEMO_V11
#define     HW_VER			HW_VER_V11
//#define     HW_VER			HW_VER_V12	

#define USE_BT1_MODULE		(1<<0)
#define USE_BT2_MODULE		(1<<1)
#define USE_BT3_MODULE		(1<<2)
#define USE_BT4_MODULE		(1<<3)

//�˺����ã����ݾ����Ӳ��ʵ��ʹ���Ǽ�������ģ����������
//#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4������ģ�鶼ͬʱ����
#if(HW_VER == HW_VER_V12)
#define BT_MODULE_CONFIG	(USE_BT1_MODULE)		//1������ģ�鶼ͬʱ����
#elif(HW_VER == HW_VER_V11)
//#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE)		//2������ģ�鶼ͬʱ����
#define BT_MODULE_CONFIG	(USE_BT1_MODULE)		//1������ģ�鶼ͬʱ����
#else
#define BT_MODULE_CONFIG	(USE_BT1_MODULE|USE_BT2_MODULE|USE_BT3_MODULE|USE_BT4_MODULE)		//4������ģ�鶼ͬʱ����
#endif

//����ģ������
#define		USE_WBTDS01		1
#define		USE_BT816		2

//����ʹ�õ�����ģ��
#define BT_MODULE		USE_BT816

#define SMALL_MEMORY_CFG			//����С�ڴ�ģʽ�������˺���Ϊ�˾�����ʡ�ڴ棬��������Ŀ���ڴ�ռ価��ѹ����20K

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
