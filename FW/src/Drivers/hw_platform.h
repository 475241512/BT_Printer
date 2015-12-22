#ifndef _HW_PLATFORM_H_
#define _HW_PLATFORM_H_

#define		HW_VER_DEMO_V11		0		//����ʹ�õ���F103VET6��100PIN����֧��4��Ӳ������ģ�飬��Ӧ��ԭ��ͼ�ǡ�BTPrinter_DemoBoard_V1.10��
#define		HW_VER_V11			1		//����ʹ�õ���F103RCT6��64PIN��, ֻ֧��2��Ӳ������ģ�飬��Ӧ��ԭ��ͼ�ǡ�BTPrinter_V1.10��

//#define     HW_VER			HW_VER_DEMO_V11
#define     HW_VER			HW_VER_V11

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

void hw_platform_init(void);
unsigned int hw_platform_USBcable_Insert_Detect(void);

#endif
