/******************** (C) COPYRIGHT 2008 STMicroelectronics ********************
* File Name          : usb_endp.c
* Author             : MCD Application Team
* Version            : V2.2.1
* Date               : 09/22/2008
* Description        : Endpoint routines
********************************************************************************
* THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
* WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE TIME.
* AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY DIRECT,
* INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING FROM THE
* CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE CODING
* INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
*******************************************************************************/

/* Includes ------------------------------------------------------------------*/
#include "usb_lib.h"
#include "usb_desc.h"
#include "usb_mem.h"
#include "hw_config.h"
#include "usb_istr.h"
#include "usb_bot.h"
#include "usb_app_config.h"
#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
//#include "record_mod.h"
#include "spi_flash.h"
#include "spi_flash_config.h"
static unsigned int write_offset = 0;
#endif
#ifdef DEBUG_VER
extern unsigned char debug_buffer[];
extern unsigned int debug_cnt;
#endif
/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
#if(USB_DEVICE_CONFIG &_USE_USB_VIRTUAL_COMM_DEVICE)
u8 buffer_out[VIRTUAL_COM_PORT_DATA_SIZE];
u32 count_out = 0;
u32 count_in = 0;
#endif

#if((USB_DEVICE_CONFIG &_USE_USB_PRINTER_DEVICE)||(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE))
u8	buffer_out[PRINTER_USB_PORT_BUF_SIZE];
u32 print_data_len;
u32 count_in = 0;
#endif

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/*******************************************************************************
* Function Name  : EP3_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP3_OUT_Callback(void)
{
#if(USB_DEVICE_CONFIG &_USE_USB_VIRTUAL_COMM_DEVICE)
   //int i;
  if (g_usb_type == USB_VIRTUAL_PORT)
  {
	  count_out = GetEPRxCount(ENDP3);
	  PMAToUserBufferCopy(buffer_out,  GetEPRxAddr(ENDP3), count_out);
	  SetEPRxValid(ENDP3);

	  /*for (i = 0; i < count_out; i++)
	  {
	  PCUsart_InByte(buffer_out[i]);
	  }*/
	  //@todo....

	  //此处应用可以处理虚拟串口接收到的数据，由具体应用来修改。。。。
  }
  else
#endif

  #if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
   u16 i;
  if (g_usb_type == USB_PRINTER_HID_COMP)
  {
		i = GetEPRxCount(ENDP3);
		PMAToUserBufferCopy(hid_buffer_out, GetEPRxAddr(ENDP3), i);
		SetEPRxValid(ENDP3);
		if (hid_buffer_out[1] == 0xf9)
		{
			strcpy(hid_buffer_out+2,"HJPrinter V1.0.3");
			//usb_SendData(,64)
			UserToPMABufferCopy(hid_buffer_out, GetEPTxAddr(ENDP2), 64);
			SetEPTxCount(ENDP2, 64);
			SetEPTxValid(ENDP2);
		}
		else if (hid_buffer_out[1] == 0x3a)
		{
			if ((hid_buffer_out[2] == 0x01)&&(hid_buffer_out[3] == 0x00))	//表示文件下载开始
			{
				//if (record_count_ext(REC1BLK))
				//{
				//	record_delall(REC1BLK);
				//}
				for (i = 0; i < DOWNLOADE_FILE_SIZE/(BLOCK_ERASE_SIZE*512);i++)
				{
					spi_flash_eraseblock(DOWNLOAD_FILE_START_SECT+i*BLOCK_ERASE_SIZE*512);
				}
				write_offset = 0;
			}
			else if ((hid_buffer_out[2] == 0x05)&&(hid_buffer_out[3] == 0x00))	//表示文件下载结束
			{

			}
			else if ((hid_buffer_out[2] == 0x02)||(hid_buffer_out[2] == 0x03)||(hid_buffer_out[2] == 0x04))
			{
				//record_write(REC1BLK,hid_buffer_out+2,67);
				spi_flash_waddr(DOWNLOAD_FILE_START_SECT+write_offset,hid_buffer_out[3],hid_buffer_out+4);
				write_offset += hid_buffer_out[3];
			}       
		}

  }
  else
#endif
	  ;
}

/*******************************************************************************
* Function Name  : EP2_OUT_Callback.
* Description    : EP2 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_OUT_Callback(void)
{
#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
		if (g_usb_type == USB_PRINTER_HID_COMP)
		{
			//@todo.....//解析打印语言
			//print_data_len = USB_SIL_Read(EP1_OUT, buffer_out);
			//ringbuffer_put(&spp_ringbuf[USB_PRINT_CHANNEL_OFFSET],buffer_out,print_data_len);
			//if (ringbuffer_data_len(&spp_ringbuf[USB_PRINT_CHANNEL_OFFSET]) < USB_RING_BUFF_FULL_TH)
			//{
			//	SetEPRxStatus(EP1_OUT, EP_RX_VALID);
			//}
		}
		else
#endif
			;
}

/*******************************************************************************
* Function Name  : EP2_OUT_Callback.
* Description    : EP2 OUT Callback Routine.
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP2_OUT_Callback(void)
{
#if(USB_DEVICE_CONFIG &_USE_USB_MASS_STOARGE_DEVICE)
	if (g_usb_type == USB_MASSSTORAGE)
	{
		Mass_Storage_Out();
	}
	else
#endif
#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_DEVICE)
		if (g_usb_type == USB_PRINTER)
		{
			//@todo.....//解析打印语言
			print_data_len = USB_SIL_Read(EP2_OUT, buffer_out);
			ringbuffer_put(&spp_ringbuf[USB_PRINT_CHANNEL_OFFSET],buffer_out,print_data_len);
			if (ringbuffer_data_len(&spp_ringbuf[USB_PRINT_CHANNEL_OFFSET]) < USB_RING_BUFF_FULL_TH)
			{
				SetEPRxStatus(EP2_OUT, EP_RX_VALID);
			}
		}
		else
#endif
			;
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP1_IN_Callback(void)
{
#if(USB_DEVICE_CONFIG &_USE_USB_MASS_STOARGE_DEVICE)
	if (g_usb_type == USB_MASSSTORAGE)
	{
		Mass_Storage_In();
	}
	else
#endif

#if(USB_DEVICE_CONFIG &_USE_USB_VIRTUAL_COMM_DEVICE)
		if (g_usb_type == USB_VIRTUAL_PORT)
	{
		count_in = 0;
	}
		else
#endif
                  ;
}

/*******************************************************************************
* Function Name  : EP1_IN_Callback
* Description    :
* Input          : None.
* Output         : None.
* Return         : None.
*******************************************************************************/
void EP2_IN_Callback(void)
{
#if(USB_DEVICE_CONFIG &_USE_USB_PRINTER_HID_COMP_DEVICE)
	count_in = 0;
#endif
		;
}
/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/

