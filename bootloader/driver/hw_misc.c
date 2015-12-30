/**
 * @file hw_misc.c
 * @brief 各种杂项硬件操作驱动
 *
 * @version V0.0.1
 * @author zhongyh
 * @date 2009年12月17日
 * @note
 *      LED状态
 *      LED3  LED2  LED1
 *       灭    灭    灭        正常
 *       亮    灭    灭        GPRS模块异常
 *       灭    亮    灭        PSAM异常
 *       灭    灭    亮        2.4G读头异常
 *
 * @copy
 *
 * 此代码为深圳江波龙电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
 * 本公司以外的项目。本司保留一切追究权利。
 *
 * <h1><center>&copy; COPYRIGHT 2009 netcom</center></h1>
 */
#include "hw_misc.h"


#define KEY_FEED()  ( GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))		//PA.1

#ifdef LED_VER
/**
***************************************************************************
*@brief	初始化电源维持管脚，管脚输出高电平，维持电源输出
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
void LED_init(void)
{
		GPIO_InitTypeDef							GPIO_InitStructure;

		//LED_BLUE	-- PB.12
		RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

		GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_12;
		GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
		GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
		GPIO_Init(GPIOB, &GPIO_InitStructure);
		LED_OFF();
}

//LED的闪烁控制
void LED_blink(unsigned int cnt,unsigned int period_ms)
{
	unsigned int i;
	for (i = 0; i < cnt;i++)
	{
		LED_ON();
		delay_ms(period_ms);
		LED_OFF();
		delay_ms(period_ms);
	}
}

//LED反转
void LED_toggle(void)
{
	static int led_state = 0;
	if (led_state == 0)
	{
		LED_OFF();
	}
	else
	{
		LED_ON();
	}

	led_state ^= 0x01;
}
#endif

void key_init(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;

	//FEED_KEY	-- PA.1
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_1;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

int if_feed_key_preesed(void)
{
	unsigned int i;
	if(!KEY_FEED())
	{
		for (i=0;i < 2000;i++);	
		if(!KEY_FEED())
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	return 0;
}

/**
 * @brief     ms软延时
 * @param[in] unsigned int time 延时参数
*/
void delay_us(unsigned int time)
{    
	unsigned int i=0;  
	while(time--)
	{
		i=8;  
		while(i--) ;    
	}
}

/**
 * @brief     ms软延时
 * @param[in] unsigned int time 延时参数
*/
void delay_ms(unsigned int time)
{    
	unsigned int i=0;  
	while(time--)
	{
		i=10255; 
		while(i--) ;    
	}
}

/**
* @brief  Initialize the IO
* @return   none
*/
void platform_misc_port_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	EXTI_InitTypeDef	EXTI_InitStructure;
	NVIC_InitTypeDef	NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_GPIOB, ENABLE);

	//USB_CHK -- PA.0
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_IN_FLOATING;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
}

/**
* @brief	检测USB是否插入
* @param[in]  none
* @param[out] none
* @return     1:  插入    0: 没有插入
* @note  如果需要在中断里面去实现，那么还需要在初始化时给此检测IO分配外部中断
*		 如果在任务级查询，那么可以之际调用此函数来检测是否插入充电电源                 
*/
unsigned int hw_platform_USBcable_Insert_Detect(void)
{
	unsigned int i;
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	{
		for (i=0;i < 2000;i++);		//延时一小段时间，防止是因为抖动造成的
		if(GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_0))
		{
			return 1;
		}
		else
		{
			return 0;
		}
	}
	else
		return 0;
}



