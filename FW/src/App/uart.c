/****************************************************************************
 *   $Id:: uart.c 3648 2010-06-02 21:41:06Z usb00423                        $
 *   Project: NXP LPC11xx UART example
 *
 *   Description:
 *     This file contains UART code example which include UART
 *     initialization, UART interrupt handler, and related APIs for
 *     UART access.
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
#include "ringbuffer.h"
#include "BT816.h"
#include "Event.h"
#include "Esc_p.h"
#include "Type.h"
#include "uart.h"
#include "stm32f10x_systick.h"
#include "stm32f10x_lib.h"
#include "usb_regs.h"
#include "usb_app_config.h"
#include "Terminal_Para.h"
#define		CHANNEL_TIMEOUT_TH		200		//当某一个串口通道在200ms内没有接收到数据时，认为此通道打印任务结束，此数据待调试	

#ifdef DEBUG_VER
extern unsigned char debug_buffer[];
extern unsigned int debug_cnt;
#endif

//unsigned char		spp_rec_buffer[MAX_PRINT_CHANNEL][SPP_BUFFER_LEN];
unsigned char		spp_rec_buffer[MAX_BT_CHANNEL][SPP_BUFFER_LEN];			//蓝牙通道的环形缓冲区size为2048
unsigned char		usb_rec_buffer[USB_BUFFER_LEN];								//USB通道的环形缓冲区size为1024足够
struct ringbuffer	spp_ringbuf[MAX_PRINT_CHANNEL];

extern uint32_t systick_cnt;

extern unsigned char	data_cache_in_nvm_flag;
//extern unsigned int    data_cache_write_offset;

//unsigned int data_cache_read_offset;

//typedef struct spi_debug_t
//{
//	unsigned int addr;
//	unsigned char data;
//}SPI_DEBUG_DATA;

//unsigned char	read_data[14*1024];

unsigned char bt1_get_char(unsigned char *data)
{
	if (ringbuffer_getchar(&spp_ringbuf[BT1_MODULE],data))
	{
		if (ringbuffer_data_len(&spp_ringbuf[BT1_MODULE]) == 0)
		{
			g_param.bt1_flash_cache_read_offset = 0;
		}
		return 1;
	}
	else
	{
		if (data_cache_in_nvm_flag)
		{
			spi_flash_raddr(BT_CACHE_START_SECT+g_param.bt1_flash_cache_read_offset,1,data);
			
			g_param.bt1_flash_cache_read_offset++;
			//NVIC_SETFAULTMASK();		//保护g_param.bt1_flash_cache_write_offset变量不会被中断改写
			USART_ITConfig(USART1,USART_IT_IDLE,DISABLE); 
			if (g_param.bt1_flash_cache_read_offset == g_param.bt1_flash_cache_write_offset)
			{
				data_cache_in_nvm_flag = 0;
				set_BT1_FREE();
			}
			//NVIC_RESETFAULTMASK();
			USART_ITConfig(USART1,USART_IT_IDLE,ENABLE); 
			return 1;
		}
		else
		{
			return 0;
		}
	}
}
static unsigned char print_channel_getchar(unsigned char ch,unsigned char *c)
{
	if (ch == BT1_MODULE)
	{
		return bt1_get_char(c);
	}
	else
	{
		return ringbuffer_getchar(&spp_ringbuf[ch],c);
	}
}
/******************************************************************************
**Function name:  Getchar
**
**Description:  Get a char from the uart 0 port
**
**parameters: none
**Returned value:
**
******************************************************************************/
extern uint8_t Getchar(void)        //接收数据
{
	uint8_t c;
	unsigned int	i,delay;
	static unsigned int timeout;
	while(1)
	{
		event_proc();

		if (current_channel == -1)
		{
			//还未进入打印通道的打印会话过程
			for (i = 0; i < MAX_PRINT_CHANNEL;i++)
			{
				if(print_channel_getchar(i,&c))
				{
					timeout = 0;
					//debug_cnt = 0;
					current_channel = i;
#ifdef DEBUG_VER
					//debug_buffer[0] = c;
					//debug_cnt = 1;
#endif
					//trip1();
					if ((current_channel == BT1_MODULE)&&(g_param.bt1_flash_cache_write_offset > 0))
					{
						for(i = 0;i < 1+(g_param.bt1_flash_cache_write_offset/(BLOCK_ERASE_SIZE*512));i++)
						{
							spi_flash_eraseblock(BT_CACHE_START_SECT+i*BLOCK_ERASE_SIZE*512);
						}
						g_param.bt1_flash_cache_write_offset = 0;
					}
					return c;
				}
			}
		}
		else
		{
			//已经进入某一个打印通道的打印会话过程
			//if (ringbuffer_getchar(&spp_ringbuf[current_channel],&c))
			//if(bt1_get_char(&c))
			if(print_channel_getchar(current_channel,&c))
			{
				timeout = 0;
				//debug_cnt = 0;
				if (current_channel != USB_PRINT_CHANNEL_OFFSET)
				{
					/*if (ringbuffer_data_len(&spp_ringbuf[current_channel]) < RING_BUFF_EMPTY_TH)
					{
						set_BT_free(current_channel);
					}*/
				}
				else
				{
					if (ringbuffer_data_len(&spp_ringbuf[current_channel]) < USB_RING_BUFF_EMPTY_TH)
					{
#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_DEVICE)
						SetEPRxStatus(EP2_OUT, EP_RX_VALID);
#elif(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
						SetEPRxStatus(EP1_OUT, EP_RX_VALID);
#endif
					}
				}
#ifdef DEBUG_VER
				//debug_buffer[debug_cnt%8096] = c;
				//debug_cnt++;
#endif
				//trip2();
				return c;
			}
			else
			{
				delay = systick_cnt;		//systick中断中维持的递增计数器
				if (timeout == 0)
				{
					timeout = (delay!=0)?delay:1;
				}
				else
				{
					if (delay != timeout)
					{
						if (delay > timeout)
						{
							delay = delay - timeout;
						}
						else
						{
							delay += 0xffffffff-timeout;
						}

						if (delay > (CHANNEL_TIMEOUT_TH/10))	//systick是10ms计数的
						{
							//trip3();
							PrintBufToZero();
							current_channel = -1;
							//debug_cnt = 0;
							//for(i = 0;i < BT_DATA_CACHE_SIZE/(BLOCK_ERASE_SIZE*512);i++)
							//{
							//	spi_flash_eraseblock(BT_CACHE_START_SECT+i*BLOCK_ERASE_SIZE*512);
							//}
						}
					}
				}
			}
		}
		
	}

}

 extern uint8_t PrintBufGetchar(uint8_t *ch)
 {
	return ringbuffer_getchar(&spp_ringbuf[current_channel],ch);
 }

 extern void PrintBufPushBytes(uint8_t c)
 {
      ringbuffer_putchar(&spp_ringbuf[current_channel],c);
 }

//======================================================================================================
extern void PrintBufPushLine( uint8_t *p, uint32_t len)
{
	ringbuffer_put(&spp_ringbuf[current_channel],p,len);
}
extern void PrintBufToZero(void)
{
     ringbuffer_reset(&spp_ringbuf[current_channel]);
	 set_BT_free(current_channel);
}
/******************************************************************************
**                            End Of File
******************************************************************************/
