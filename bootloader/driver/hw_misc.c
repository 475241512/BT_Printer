/**
 * @file hw_misc.c
 * @brief ��������Ӳ����������
 *
 * @version V0.0.1
 * @author zhongyh
 * @date 2009��12��17��
 * @note
 *      LED״̬
 *      LED3  LED2  LED1
 *       ��    ��    ��        ����
 *       ��    ��    ��        GPRSģ���쳣
 *       ��    ��    ��        PSAM�쳣
 *       ��    ��    ��        2.4G��ͷ�쳣
 *
 * @copy
 *
 * �˴���Ϊ���ڽ������������޹�˾��Ŀ���룬�κ��˼���˾δ����ɲ��ø��ƴ�����������
 * ����˾�������Ŀ����˾����һ��׷��Ȩ����
 *
 * <h1><center>&copy; COPYRIGHT 2009 netcom</center></h1>
 */
#include "hw_misc.h"


#define KEY_FEED()  ( GPIO_ReadInputDataBit(GPIOA,GPIO_Pin_1))		//PA.1

#ifdef LED_VER
/**
***************************************************************************
*@brief	��ʼ����Դά�ֹܽţ��ܽ�����ߵ�ƽ��ά�ֵ�Դ���
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

//LED����˸����
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

//LED��ת
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
 * @brief     ms����ʱ
 * @param[in] unsigned int time ��ʱ����
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
 * @brief     ms����ʱ
 * @param[in] unsigned int time ��ʱ����
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
* @brief	���USB�Ƿ����
* @param[in]  none
* @param[out] none
* @return     1:  ����    0: û�в���
* @note  �����Ҫ���ж�����ȥʵ�֣���ô����Ҫ�ڳ�ʼ��ʱ���˼��IO�����ⲿ�ж�
*		 ��������񼶲�ѯ����ô����֮�ʵ��ô˺���������Ƿ�������Դ                 
*/
unsigned int hw_platform_USBcable_Insert_Detect(void)
{
	unsigned int i;
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_0))
	{
		for (i=0;i < 2000;i++);		//��ʱһС��ʱ�䣬��ֹ����Ϊ������ɵ�
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



