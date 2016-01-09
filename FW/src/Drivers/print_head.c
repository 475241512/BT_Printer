/**
 * @file  print_head.c
 * @brief 蓝牙打印机项目的热敏打印头管理模块
 * @version 1.0
 * @author kent.zhou
 * @date 2015年11月12日
 * @note
*/
#include "Type.h"
#include "print_head.h"
#include "TP.h"
#include "PaperDetect.h"
#include "stm32f10x_lib.h"
#include "TimeBase.h"
#include <assert.h>
#include "hw_platform.h"

/**
* @brief	初始化热敏打印头的SPI接口
* @return     none
* @note                    
*/
static void print_head_SPI_init(void)
{
	GPIO_InitTypeDef	GPIO_InitStructure;
	SPI_InitTypeDef		SPI_InitStructure;

	/* Enable SPI1 and GPIO clocks */
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SPI1 | RCC_APB2Periph_GPIOA, ENABLE);

	/* Configure SPI1 pins: SCK and MOSI */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	/* SPI1 configuration */
	SPI_InitStructure.SPI_Direction = SPI_Direction_1Line_Tx;
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_16;			//由于热敏打印头支持的SPI通讯频率最大为10MHZ，所以此处设置为16分频，4.5M比较安全
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI1, &SPI_InitStructure);

#if((HW_VER == HW_VER_V11)||(HW_VER == HW_VER_V12))
	//由于Demo V1.1的SPI接口的DMA通道被串口3占用，而此版本可以用DMA来进行发送。
	DMA_InitTypeDef					DMA_InitStructure;

	/* DMA clock enable */
		RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);

		/* fill init structure */
		DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;
		DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;
		DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_Byte;
		DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_Byte;
		DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;
		DMA_InitStructure.DMA_Priority = DMA_Priority_VeryHigh;
		DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;

		/* DMA1 Channel3 (triggered by SPI1 Tx event) Config */
		DMA_DeInit(DMA1_Channel3);
		DMA_InitStructure.DMA_PeripheralBaseAddr =(u32)(&SPI1->DR);
		DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;
		/* As we will set them before DMA actually enabled, the DMA_MemoryBaseAddr
		* and DMA_BufferSize are meaningless. So just set them to proper values
		* which could make DMA_Init happy.
		*/
		DMA_InitStructure.DMA_MemoryBaseAddr = (u32)0;
		DMA_InitStructure.DMA_BufferSize = 1;
		DMA_Init(DMA1_Channel3, &DMA_InitStructure);

		DMA_Cmd(DMA1_Channel3,ENABLE);  

		/* Enable SPI1 DMA Tx request */
		SPI_I2S_DMACmd(SPI1,SPI_I2S_DMAReq_Tx,ENABLE); 

#endif

	/* Enable SPI1  */
	SPI_Cmd(SPI1, ENABLE);
}


/**
* @brief	初始化热敏打印头相关的控制IO、SPI接口、定时器及AD模块
* @return     none
* @note                    
*/
void print_head_init(void)
{
	TPInit();
	TPPaperSNSInit();
	print_head_SPI_init();
	SetDesity();		//设置打印速度，表现为打印字体的密度
}

/**
* @brief	通过SPI接口发送数据到热敏打印头
* @return     none
* @note                    
*/
void print_head_spi_send_data(unsigned char *data,unsigned int len)
{
#if((HW_VER == HW_VER_V11)||(HW_VER == HW_VER_V12))
	///* disable DMA */
	//DMA_Cmd(DMA1_Channel3, DISABLE);

	///* set buffer address */
	////MEMCPY(BT816_send_buff[BT1_MODULE],pData,length);

	//DMA1_Channel3->CMAR = (u32)data;
	///* set size */
	//DMA1_Channel3->CNDTR = len;

	//SPI_I2S_DMACmd(SPI1, SPI_I2S_DMAReq_Tx , ENABLE);
	///* enable DMA */
	//DMA_Cmd(DMA1_Channel3, ENABLE);
	//while(DMA1_Channel3->CNDTR);
	//while(SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET){};

	unsigned int k;
	volatile short			i = 0;
	for (k = 0; k < len; k++)
	{
		/* Loop while DR register in not emplty */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

		/* Send byte through the SPI2 peripheral */
		SPI_I2S_SendData(SPI1, data[k]);

		/* Wait to receive a byte */
		//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		//for(i=0; i<10; i++);
		for(i=0; i<5; i++);
	}
#else
	unsigned int k;
	volatile short			i = 0;
	for (k = 0; k < len; k++)
	{
		/* Loop while DR register in not emplty */
		while (SPI_I2S_GetFlagStatus(SPI1, SPI_I2S_FLAG_TXE) == RESET);

		/* Send byte through the SPI2 peripheral */
		SPI_I2S_SendData(SPI1, data[k]);

		/* Wait to receive a byte */
		//while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET);
		//for(i=0; i<10; i++);
		for(i=0; i<5; i++);
	}
#endif
	return;
}


