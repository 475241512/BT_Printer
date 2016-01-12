/**
* @file main.c
* @brief 蓝牙打印机项目主程序
*
* @version V0.0.1
* @author joe
* @date 2015年11月12日
* @note
*		none
*
* @copy
*
* 此代码为深圳合杰电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2015 heroje</center></h1>
*/

/* Private Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include <string.h>
#include <stdlib.h>
#include "hw_platform.h"
#include "data_uart.h"
#include "TimeBase.h"
#include "Terminal_para.h"
#include "usb_app_config.h"
#include "res_spi.h"
#include "Event.h"
#include "record_mod.h"
/* Private define ------------------------------------------------------------*/

// Cortex System Control register address
#define SCB_SysCtrl					((u32)0xE000ED10)
// SLEEPDEEP bit mask
#define SysCtrl_SLEEPDEEP_Set		((u32)0x00000004)

#ifdef DEBUG_VER
unsigned char	debug_buffer[2];
unsigned int	debug_cnt;
#endif

#define		IAP_SIZE					(1024*16)	//BootCode Size

/* Global variables ---------------------------------------------------------*/
ErrorStatus			HSEStartUpStatus;							//Extern crystal OK Flag


/* Private functions ---------------------------------------------------------*/
static void Unconfigure_All(void);
static void GPIO_AllAinConfig(void);
void RCC_Configuration(void);

/* External variables -----------------------------------------------*/
extern	TTerminalPara			g_param;					//Terminal Param

/*******************************************************************************
* Function Name  : system_error_tip
* Description    : 严重的系统错误提示
*******************************************************************************/
void system_error_tip(int err_no)
{
	unsigned char str[15];
#ifdef LCD_VER
	Lcd_clear(1);
	Lcd_TextOut(0,12,"System Err");
	//sprintf(str,"Code:%d",err_no);
	STRNCPY(str,"Code:",5);
	hex_to_str(err_no,0x10,0,str+5);
	Lcd_TextOut(0,24,str);
#endif
	//@todo...
	while(1)
	{
#ifdef LCD_VER
		Lcd_blink(1,120);
#else
		LED_blink(1,60);
#endif
	}
}

#if(USB_DEVICE_CONFIG &_USE_USB_MASS_STOARGE_DEVICE)
/*******************************************************************************
* Function Name  : enter_u_disk_mode
* Description    : 进入U盘模式
*******************************************************************************/
void enter_u_disk_mode(void)
{
	int key_state = 0;
	g_mass_storage_device_type = MASSTORAGE_DEVICE_TYPE_SPI_FLASH;
	usb_device_init(USB_MASSSTORAGE);
	//usb_Cable_Config(ENABLE);
#ifdef LCD_VER
	Lcd_clear(1);
	Lcd_TextOut(0,12,"UDisk Mode");
#endif

	while(1)
	{
#ifdef LCD_VER
		Lcd_blink(1,200);
#else
		LED_blink(1,200);
#endif
		if(!KEY_FEED())
		{
			key_state++;
		}
		else
		{
			key_state = 0;
		}
		if( key_state == 5)		//退出USB模式，系统复位
		{
			//usb_Cable_Config(DISABLE);
			NVIC_SETFAULTMASK();
			NVIC_GenerateSystemReset();
		}
	}
}
#endif

/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
	int	ret,key_state;
	/* System Clocks Configuration **********************************************/
	RCC_Configuration(); 
#ifdef RELEASE_VER
	/* NVIC Configuration *******************************************************/
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, IAP_SIZE);		//需要加密的 bootcode
#else	
	/* NVIC Configuration *******************************************************/
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);
#endif

	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);		//定义整个系统的优先级分组为Group1，1位抢占优先级（2个级别的中断嵌套）	3位响应优先级
	
	// Clear SLEEPDEEP bit of Cortex System Control Register
	*(vu32 *) SCB_SysCtrl &= ~SysCtrl_SLEEPDEEP_Set;

	Unconfigure_All();
	// 数据串口(调试口)初始化
	data_uart_init();
       
	spi_flash_init();
        //while(1);

#ifdef DEBUG_VER
	//MEMSET(debug_buffer,0,8000*2);
	//debug_cnt = 0;

	//printf("BTPrinter startup...\r\n");
	//if (HSEStartUpStatus == SUCCESS)
	//{
	//	printf("HSE OK!\r\n");
	//}
	//else
	//{
	//	printf("HSE Failed!\r\n");
	//}
        debug_buffer[0] = 0;
#endif

	//初始化时基函数
	TimeBase_Init();


	//初始化按键模块
	KeyScanInit();

	//box_ctrl(100);

	key_state = 0;
	if (!KEY_FEED())
	{
		key_state = 1;
	}

	hw_platform_init();

#ifdef LCD_VER
	Lcd_dispBMP(PIC_POWERON);
	//Lcd_clear(1);
	//Lcd_DrawLineH(0,12,64,0);
	//Lcd_DrawLineH(0,12,64,1);
	//Lcd_DrawLineH(0,16,64,0);
	//Lcd_DrawLineH(0,16,64,1);
	//Lcd_DrawLineV(24,0,48,0);
	//Lcd_DrawLineV(24,0,48,1);
	//Lcd_DrawLineV(24,4,30,0);
	//Lcd_DrawLineV(24,4,30,1);
	//Lcd_TextOut(0,0,"Test01234");
	//Lcd_setfont(FONT_16x8);
	//Lcd_TextOut(0,12,"Test0123456");
	//Lcd_setfont(FONT_12x6);
	//Lcd_TextOut(2,28,"Test0123456");
#endif

	if (!KEY_FEED())
	{
		if (key_state == 1)
		{
			key_state = 2;
		}
	}
 
	//初始化参数模块（SPI FLASH）
	ret = ReadTerminalPara();
	if (ret)
	{
		if (ret > 0 || ret == -4)
		{
			ret = DefaultTerminalPara(); 
			if (ret)
			{
				system_error_tip(ret);	//严重的错误，系统停止继续运行，可能进入诊断模式
			}
		}
		else
		{
			system_error_tip(ret);	//严重的错误，系统停止继续运行，可能进入诊断模式
		}
	}

	event_init();
	
	//检查系统的字库资源是否正确
#if(USB_DEVICE_CONFIG &_USE_USB_MASS_STOARGE_DEVICE)
	res_upgrade();
#endif

#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
	//实现了Printer+HID的复合设备时，采用HID接口将升级文件download到SPI Flash保存起来
	//利用record_mod驱动来保存下载下来的Intel Hex文件，重新上电后，进入Bootloader再将Intel Hex格式的升级文件解析并编程到相应的FLASH空间
	//每条记录存储了3字节的冗余信息，TAG(1BYTE)+LEN[1BYTE]+[DATA(0),DATA(1)，...,DATA(n)](LEN BYTE)+CHECKSUM[1BYTE]
	ret = record_init(REC1BLK,67,256*1024/64);	//最多支持256K的BIN文件的下载，只开辟这么大的空间来保存HEX文件
	if (ret != 0)
	{
#ifdef DEBUG_VER
		//bootloader负责完成此记录的初始化
		if (ret == -3 || ret == -4 || ret == -6)
		{
			ret = record_format(REC1BLK,67,256*1024/64);
			if (ret)
			{
				system_error_tip(80);
			}
		}
		else
#endif
		{
			system_error_tip(81);
		}
	}

	ret = record_count(REC1BLK);
	if (ret < 0)
	{
		system_error_tip(81);
	}
	else if(ret > 0)
	{
		if(record_delall(REC1BLK))
		{
			system_error_tip(81);
		}
	}
#endif

	ret = res_init();
	if (ret != 0)
	{
#if(USB_DEVICE_CONFIG &_USE_USB_MASS_STOARGE_DEVICE)
		enter_u_disk_mode();
#else
		system_error_tip(82);
#endif
	}
	else
	{
		if (key_state == 2)
		{
			//应用在开机时不需要接口进入升级模式，由bootloader负责提供接口强制进入升级模式
			//if (usb_cable_insert())
			//{
			//	enter_u_disk_mode();
			//}
			//else
			{
				event_post(evtKeyDownHold2000msMode);
			}
		}
	}

	//初始化热敏打印头的控制IO及定时器
	print_head_init();

	//初始化蓝牙模块
	if(BT816_init())
	{
		system_error_tip(83);
	}

	SaveTerminalPara();

	esc_init();

#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
	usb_device_init(USB_PRINTER_HID_COMP);
#elif(USB_DEVICE_CONFIG & _USE_USB_PRINTER_DEVICE)
	usb_device_init(USB_PRINTER);
#endif
	//test_motor();

	PaperStartSns();		//Systick跳动起来


	while (1)
	{
		esc_p();		//等待串口接收到的数据，根据不同的数据进行相应的处理
    }
}

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
void RCC_Configuration(void)
{   
	vu32 i=0;

	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE							*/
	RCC_HSEConfig(RCC_HSE_ON);
	// 这里要做延时，才能兼容某些比较差的晶体，以便顺利起震	
	for(i=0; i<200000; i++);

	/* Wait till HSE is ready			*/
	HSEStartUpStatus = RCC_WaitForHSEStartUp();

	if(HSEStartUpStatus == SUCCESS)
	{
		/* Enable Prefetch Buffer		*/
		FLASH_PrefetchBufferCmd(FLASH_PrefetchBuffer_Enable);

		/* Flash 2 wait state			*/
		FLASH_SetLatency(FLASH_Latency_2);

		/* HCLK = SYSCLK					*/
		RCC_HCLKConfig(RCC_SYSCLK_Div1); 

		/* PCLK2 = HCLK					*/
		RCC_PCLK2Config(RCC_HCLK_Div1); 

		/* PCLK1 = HCLK/2					*/
		RCC_PCLK1Config(RCC_HCLK_Div2);

		
#ifdef GD_MCU
		/* PLLCLK = 12MHz * 8 = 96 MHz	*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
#else
		/* PLLCLK = 12MHz * 6 = 72 MHz	*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);
#endif
		/* PLLCLK = 8MHz * 9 = 72 MHz	*/
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_9);

		/* Enable PLL						*/
		RCC_PLLCmd(ENABLE);

		/* Wait till PLL is ready		*/
		while(RCC_GetFlagStatus(RCC_FLAG_PLLRDY) == RESET)
		{
		}

		/* Select PLL as system clock source */
		RCC_SYSCLKConfig(RCC_SYSCLKSource_PLLCLK);

		/* Wait till PLL is used as system clock source */
		while(RCC_GetSYSCLKSource() != 0x08)
		{
		}
	}
}

/*******************************************************************************
* Function Name  : Unconfigure_All
* Description    : set all the RCC data to the default values 
*                  configure all the GPIO as input
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void Unconfigure_All(void)
{
	//RCC_DeInit();

	/* RCC configuration */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ALL, DISABLE);
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_ALL, DISABLE);

	GPIO_AllAinConfig();
}


/*******************************************************************************
* Function Name  : GPIO_AllAinConfig
* Description    : Configure all GPIO port pins in Analog Input mode 
*                  (floating input trigger OFF)
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void GPIO_AllAinConfig(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	/* Configure all GPIO port pins in Analog Input mode (floating input trigger OFF) */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_All;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	GPIO_Init(GPIOB, &GPIO_InitStructure);
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	GPIO_Init(GPIOD, &GPIO_InitStructure);
	GPIO_Init(GPIOE, &GPIO_InitStructure);
}

/************************************************
* Function Name  : EnterLowPowerMode()
************************************************/
void EnterLowPowerMode(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//BT816_enter_sleep();
	stop_real_timer();

	//但是需要开启PWR模块的时钟
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR, ENABLE); // Enable PWR clock

	// enable Debug in Stop mode
	//DBGMCU->CR |= DBGMCU_CR_DBG_STOP;
	
	//进入低功耗模式
	EXTI_ClearFlag(0xffff);
	PWR_EnterSTOPMode(PWR_Regulator_ON, PWR_STOPEntry_WFI);
}

/************************************************
* Function Name  : ExitLowPowerMode()
************************************************/
void ExitLowPowerMode(void)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	//重新配置时钟
	RCC_Configuration();
	//BT816_wakeup();
	start_real_timer();
}

//关闭某一个中断
 void NVIC_DisableIRQ(unsigned char	irq_channel)
{
	NVIC_InitTypeDef							NVIC_InitStructure;
	/* Enable the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= irq_channel;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= DISABLE;
	NVIC_Init(&NVIC_InitStructure);
}

//使能某一个中断
 void NVIC_EnableIRQ(unsigned char	irq_channel)
{
	NVIC_InitTypeDef							NVIC_InitStructure;
	/* Enable the TIM3 Interrupt */
	NVIC_InitStructure.NVIC_IRQChannel			= irq_channel;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority	= 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd		= ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}



/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/

//void assert_failed(u8* file, u32 line)
//{ 
/* User can add his own implementation to report the file name and line number,
ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

/* Infinite loop */
//while (1)
//{
//}
//}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

