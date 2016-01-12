/**
* @file main.c
* @brief											惠州售货机 bootloader
*
*
* @version V0.0.1
* @author xuehui869
* @date 2010年5月21日14:17:10
* @note
*      项目概况:
*      
*
* @copy
*
* 此代码为深圳江波龙电子有限公司项目代码，任何人及公司未经许可不得复制传播，或用于
* 本公司以外的项目。本司保留一切追究权利。
*
* <h1><center>&copy; COPYRIGHT 2010 netcom</center></h1>
*/

/* Includes ------------------------------------------------------------------*/
#include "stm32f10x_lib.h"
#include <string.h>
#include "stm32f10x_flash.h"
#include "des.H"
#include "usb_lib.h"
#include "usb_app_config.h"
#include "hw_misc.H"

#if(ENCRYPT_MODE == ENCRYPT_MODE_KT_XOR)
const char *KT_key="joe3501@foxmail.com";
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
#include "ff.h"

#define update_file					"/update.bin"
FIL									file;
FATFS								ffat;				// 

#define FLASH_SIZE					(128*1024)				//128K
#define IAP_SIZE					(32*1024)				// IAP的升级大小

#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)
//#include "record_mod.h"
#include "spi_flash_config.h"
//此种升级方式支持更小容量的主控
#define FLASH_SIZE					(64*1024)				//64K
#define IAP_SIZE					(16*1024)				//BootCode的大小,留给应用的空间只有48K了

#define TAG_BIN_DATA	0x02
#define TAG_FNT_DATA	0x03
#define TAG_PIC_DATA	0x04

#endif

unsigned char						ProHBuffer[2048];		//最大2048/8 =256个包
unsigned char						buffer_org[512];
int									code_sector;			// 代码长度，单位扇区
unsigned int						code_xor;


#define  SUCCESS					1
#define  FAIL						0


unsigned char key1[8]				= {0xA8,0xB2,0xAD,0xBD,0xDA,0xDB,0xEE,0xC9};	// "深圳江波"
unsigned char key2[8]				= {0x20,0x20,0xB7,0xC6,0xF6,0xB3,0xFA,0xC1};	// "龙出品  "


/* Private function prototypes -----------------------------------------------*/
static void RCC_Configuration(void);


unsigned int						JumpAddress;

#define FLASH_START					0x08000000				// Flash起始地址
#define ApplicationAddress			(FLASH_START+IAP_SIZE)
#define ApplicationSize				(FLASH_SIZE-IAP_SIZE)
typedef void (*pFunction)(void);
pFunction							Jump_To_Application;

typedef struct  
{
	unsigned char					magic[4];
	unsigned int					xor;
	unsigned int					xor_data;
	unsigned int					length;
	unsigned char					OEMName[16];
	unsigned char					Version[16];
	unsigned char					Date[16];
}TPackHeader;

/**
***************************************************************************
*@brief	向特定的地址进行编程
*@param[in] addr 此地址为应用区域的地址，从0开始，单位为字节
*@param[in] *pdata 写入的数据
*@param[in] len 写入数据的长度，单位为字，即 len*4 字节
*@return -1: 编程写入失败
*         0: 编程成功
*@warning
*@see	
*@note 
***************************************************************************
*/
static int prog_data(unsigned int addr, unsigned int *pdata, int len)
{

	while(len--)
	{
		if( FLASH_ProgramWord(addr, *pdata) != FLASH_COMPLETE )
		{
			return -1;
		}

		if( *pdata != *((unsigned int*)addr))
		{
			return -1;
		}

		pdata ++;
		addr						+= 4;
		//if (len%96 == 0)
		//{
		//	LED_toggle();
		//}
	}

	return 0;
}

/**
***************************************************************************
*@brief	校验flash中的数据是否和
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
static int verify_data(unsigned int addr, unsigned int *pdata, int len)
{
	while( len -- )
	{
		if( *pdata != *((unsigned int *)addr))
		{
			return -1;
		}
	}

	return 0;
}
#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)
//static unsigned int record_offset = 0;
//static int record_sector_read(unsigned char *read_buf,unsigned int sector_cnt,unsigned int *real_byte_cnt)
//{
//	int ret,i,j;
//	unsigned char buf[67];
//	unsigned char len,check_value=0;
//	*real_byte_cnt = 0;
//	for (i = 0; i < sector_cnt*512/64;i++)
//	{
//		ret = sp(REC1BLK,record_offset,buf,0);
//		if (ret)
//		{
//			return ret;
//		}
//
//		if (buf[0] != TAG_BIN_DATA)
//		{
//			return 1;
//		}
//
//		len = buf[1];
//		if ((len == 0)||(len > 64))
//		{
//			return 2;
//		}
//
//		//checksum校验，
//		for (j = 0; j < len+2;j++)
//		{
//			check_value += buf[j];
//		}
//
//		check_value %= 256;
//		check_value = ~check_value;
//		check_value += 1;
//
//		if (check_value != buf[len+2])
//		{
//			return 3;
//		}
//
//		memcpy(read_buf+len*i,&buf[2],len);
//
//		(*real_byte_cnt) += len;
//
//		record_offset++;
//	}
//
//	return 0;
//}
static unsigned int read_offset = 0;
#endif
/**
***************************************************************************
*@brief	
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
static int check_updatefile(void)
{
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	UINT							rd;
	int								i;
	int								j;
	unsigned int					xor;

	//unsigned char	tmp_key1[8]={0};
	//unsigned char	tmp_key2[8]={0};

	f_mount(0, &ffat);			//先挂载
	if( f_open(&file, update_file, FA_OPEN_EXISTING | FA_READ ) != FR_OK )
	{
		return -1;
	}

	if( f_read(&file, ProHBuffer, 512, &rd) != FR_OK )
	{
		f_close(&file);
		return -1;
	}

	if( ProHBuffer[0] != 'J' || ProHBuffer[1] != 'B' || ProHBuffer[2] != 'L' )
	{
		f_close(&file);
		return -1;
	}

	//check oem_name 字符串
	if (strcmp(((TPackHeader*)&ProHBuffer[0])->OEMName,"HJBTPrinter"))
	{
		f_close(&file);
		return -1;
	}

	code_sector						= ((TPackHeader*)ProHBuffer)->length;
	code_xor						= ((TPackHeader*)ProHBuffer)->xor_data;

	code_sector						/= 512;
	xor								= 0;
	for(i=0; i<code_sector; i++)
	{
		if( f_read(&file, ProHBuffer, 512, &rd) != FR_OK )
		{
			f_close(&file);
			return -1;
		}

		for(j=0; j<128; j++)
		{
			xor	^= *((unsigned int*)&ProHBuffer[j*4]);
		}
	}


	if(xor != code_xor)
	{
		f_close(&file);
		return -1;
	}

	f_close(&file);
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)
	int i,j,ret,cnt;
        unsigned int					xor;
	unsigned int real_byte_cnt;
	//ret = record_init(REC1BLK,67,256*1024/64);	//最多支持256K的BIN文件的下载，只开辟这么大的空间来保存HEX文件
	//if (ret != 0)
	//{
	//	if (ret == -3 || ret == -4 || ret == -6)
	//	{
	//		ret = record_format(REC1BLK,67,256*1024/64);
	//		if (ret)
	//		{
	//			return ret;
	//		}
	//	}
	//	else
	//	{
	//		return ret;
	//	}
	//}

	//cnt = record_count(REC1BLK); 
	//if ((cnt < 512/64)||(cnt > (ApplicationSize+512)/64))
	//{
	//	return 1;
	//}

	//if( spi_flash_read(ProHBuffer,1,&real_byte_cnt) != 0)
	//{
	//	return 2;
	//}

	//if (real_byte_cnt != 512)
	//{
	//	return 2;
	//}

	read_offset = 0;
	if (spi_flash_read(DOWNLOAD_FILE_START_SECT/512,ProHBuffer,1))
	{
		return 1;
	}

	read_offset += 1;

	if( ProHBuffer[0] != 'J' || ProHBuffer[1] != 'B' || ProHBuffer[2] != 'L' )
	{
		return 3;
	}

	//check oem_name 字符串
	if (strcmp(((TPackHeader*)&ProHBuffer[0])->OEMName,"HJBTPrinter"))
	{
		return 4;
	}

	code_sector						= ((TPackHeader*)ProHBuffer)->length;
	code_xor						= ((TPackHeader*)ProHBuffer)->xor_data;

	code_sector						/= 512;
	xor								= 0;
	for(i=0; i<code_sector; i++)
	{
		//if( record_sector_read(ProHBuffer,1,&real_byte_cnt) != 0)
		//{
		//	return 5;
		//}

		//if (real_byte_cnt != 512)
		//{
		//	return 5;
		//}

		if (spi_flash_read(DOWNLOAD_FILE_START_SECT/512+read_offset,ProHBuffer,1))
		{
			return 2;
		}

		read_offset ++;

		for(j=0; j<128; j++)
		{
			xor	^= *((unsigned int*)&ProHBuffer[j*4]);
		}
	}


	if(xor != code_xor)
	{
		return 6;
	}

#endif
	return 0;
}

/**
***************************************************************************
*@brief	升级应用程序
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
int update_app(void)
{
	unsigned int					total_sector;
	unsigned int							rd;
	int								i;
	unsigned int					xor;
	unsigned int					addr_offset,real_byte_cnt;
	int								update_index;
	//unsigned char	str[5];
	//unsigned char	len;
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if( f_open(&file, update_file, FA_OPEN_EXISTING | FA_READ ) != FR_OK )
	{
		return -1;
	}

	if( f_read(&file, ProHBuffer, 512, &rd) != FR_OK )
	{
		f_close(&file);
		return -1;
	}

	total_sector					= ((TPackHeader*)ProHBuffer)->length;
	total_sector					/= 512;

#ifdef LCD_VER
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
#endif
	//
	FLASH_Unlock();

	for(i=0; i< (ApplicationSize/1024)*2; i++)			
	{
		FLASH_ErasePage( ApplicationAddress + i * 512 );
		if (i%2==0)
		{
#ifdef LCD_VER
			Lcd_process_bar_step();
#else
			LED_toggle();
#endif
		}
	}

	xor								= 0;
	addr_offset						= 0;

	//while(total_sector--)

	// 重新计算密钥
	for(i=0; i<8; i++)
	{
		key2[i]						^= key1[i];
	}
	for(i=0; i<8; i++)
	{
		key1[i]						^= 0xD6;
	}


	for(update_index=0; update_index<total_sector; update_index++)
	{
		// 读数据
		if( record_sector_read(ProHBuffer,1,&real_byte_cnt) != 0)
		{
			goto update_fail;
			return 2;
		}

		if (real_byte_cnt != 512)
		{
			goto update_fail;
			return 2;
		}

		// 计算校验：异或
		for(i=0; i<128; i++)
		{
			xor						^= *((unsigned int*)&ProHBuffer[i*4]);
		}
#ifdef LCD_VER
		Lcd_process_bar_step();
#else
        LED_toggle();
#endif
#if(ENCRYPT_MODE == ENCRYPT_MODE_3DES)
		// 3DES解密
		un3DES(ProHBuffer, 512, key1, key2, key1, buffer_org, &rd);
#elif(ENCRYPT_MODE == ENCRYPT_MODE_DES)
		//DES 解密
		un3DES(ProHBuffer, 512, key1, buffer_org, &rd);
#elif(ENCRYPT_MODE == ENCRYPT_MODE_KT_XOR)
		//KT_Xor解密
		decode_KT_Xor(ProHBuffer,512,(unsigned char*)KT_key,read_offset-2,buffer_org,&rd);
#else
		//明文
		MEMCPY(ProHBuffer,buffer_org,512);
#endif

#ifdef LCD_VER
		Lcd_process_bar_step();
#else
		LED_toggle();
#endif
		// 编程写入
		if( prog_data( ApplicationAddress + addr_offset, (unsigned int*)buffer_org, 128 ) != 0 )
		{
			goto update_fail;
		}

		// 校验写入
		if( verify_data( ApplicationAddress + addr_offset, (unsigned int*)buffer_org, 128) != 0 )
		{
			goto update_fail;
		}

		addr_offset					+= 512;

		//LED_toggle();
	}


	// 在程序的结尾，写个0x12345678的标号
	i								= 0x12345678UL;
	prog_data(ApplicationAddress + ApplicationSize - 4, &i, 1);
	FLASH_Lock();

#ifdef LED_VER
	LED_ON();
#else
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
	Lcd_TextOut(26,24,"OK");
	delay_ms(2000);
#endif
	// ==========================================================================================================

	return 0;

update_fail:

	// erase first block;
	FLASH_ErasePage(ApplicationAddress);
	FLASH_Lock();
#ifdef LED_VER
	LED_OFF();
#else
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
	Lcd_TextOut(26,24,"Fail");
	delay_ms(2000);
#endif
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)

	//record_offset = 0;

	//if( record_sector_read(ProHBuffer,1,&real_byte_cnt) != 0)
	//{
	//	return 2;
	//}

	//if (real_byte_cnt != 512)
	//{
	//	return 2;
	//}

	read_offset = 0;
	if (spi_flash_read(DOWNLOAD_FILE_START_SECT/512,ProHBuffer,1))
	{
		return 1;
	}

	read_offset++;

	total_sector					= ((TPackHeader*)ProHBuffer)->length;
	total_sector					/= 512;
        
#ifdef LCD_VER
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
#endif
	//
	FLASH_Unlock();

	for(i=0; i< (ApplicationSize/1024)*2; i++)			
	{
		FLASH_ErasePage( ApplicationAddress + i * 512 );
		if (i%2==0)
		{
#ifdef LCD_VER
			Lcd_process_bar_step();
#else
			LED_toggle();
#endif
		}
	}

	xor								= 0;
	addr_offset						= 0;

	//while(total_sector--)

	// 重新计算密钥
	for(i=0; i<8; i++)
	{
		key2[i]						^= key1[i];
	}
	for(i=0; i<8; i++)
	{
		key1[i]						^= 0xD6;
	}


	for(update_index=0; update_index<total_sector; update_index++)
	{
		// 读数据
		//if( record_sector_read(ProHBuffer,1,&real_byte_cnt) != 0)
		//{
		//	goto update_fail;
		//	return 2;
		//}

		//if (real_byte_cnt != 512)
		//{
		//	goto update_fail;
		//	return 2;
		//}

		if (spi_flash_read(DOWNLOAD_FILE_START_SECT/512+read_offset,ProHBuffer,1))
		{
			goto update_fail;
			return 2;
		}

		read_offset++;

		// 计算校验：异或
		for(i=0; i<128; i++)
		{
			xor						^= *((unsigned int*)&ProHBuffer[i*4]);
		}
#ifdef LCD_VER
		Lcd_process_bar_step();
#else
		LED_toggle();
#endif
#if(ENCRYPT_MODE == ENCRYPT_MODE_3DES)
		// 3DES解密
		un3DES(ProHBuffer, 512, key1, key2, key1, buffer_org, &rd);
#elif(ENCRYPT_MODE == ENCRYPT_MODE_DES)
		//DES 解密
		un3DES(ProHBuffer, 512, key1, buffer_org, &rd);
#elif(ENCRYPT_MODE == ENCRYPT_MODE_KT_XOR)
		//KT_Xor解密
		decode_KT_Xor(ProHBuffer,512,(unsigned char*)KT_key,read_offset-2,buffer_org,&rd);
#else
		//明文
		MEMCPY(ProHBuffer,buffer_org,512);
#endif
#ifdef LCD_VER
		Lcd_process_bar_step();
#else
		LED_toggle();
#endif
		// 编程写入
		if( prog_data( ApplicationAddress + addr_offset, (unsigned int*)buffer_org, 128 ) != 0 )
		{
			goto update_fail;
		}

		// 校验写入
		if( verify_data( ApplicationAddress + addr_offset, (unsigned int*)buffer_org, 128) != 0 )
		{
			goto update_fail;
		}

		addr_offset					+= 512;

		//LED_toggle();
	}


	// 在程序的结尾，写个0x12345678的标号
	i								= 0x12345678UL;
	prog_data(ApplicationAddress + ApplicationSize - 4, &i, 1);
	FLASH_Lock();

#ifdef LED_VER
	LED_ON();
#else
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
	Lcd_TextOut(26,24,"OK");
	delay_ms(2000);
#endif
	// ==========================================================================================================

	return 0;

update_fail:

	// erase first block;
	FLASH_ErasePage(ApplicationAddress);
	FLASH_Lock();
#ifdef LED_VER
	LED_OFF();
#else
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
	Lcd_TextOut(26,24,"Fail");
	delay_ms(2000);
#endif
#endif
	return -1;
}

/**
***************************************************************************
*@brief	检查应用程序是否有效
*@param[in] 
*@return 
*@warning
*@see	
*@note 
***************************************************************************
*/
int verify_app(void)
{
	if( *(unsigned int*)(ApplicationAddress + (76*4)) != 0x12345678uL )
		return -1;

	if( *(unsigned int*)(ApplicationAddress + (79*4)) != 0x12345678uL )
		return -1;

	if( *(unsigned int*)(ApplicationAddress + ApplicationSize - 4) != 0x12345678 )
		return -1;

	return 0;
}


/**
***************************************************************************
*@brief 判断是否有应用升级文件的存在，如果有升级文件存在就删除应用升级文件
*@param[in] 
*@return 0 删除成功  else; 删除失败
*@warning
*@see	
*@note 
***************************************************************************
*/
static int del_update_bin(void)
{

	//if (SPI_SD_Init() != SD_RESPONSE_NO_ERROR)
	//{
	//	return -1;
	//}

	//挂载文件系统
	//f_mount(0, &fs);										// 装载文件系统

#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
	if( f_open(&file, "/update.bin", FA_OPEN_EXISTING | FA_READ ) != FR_OK )
	{
		return -1;
	}

	f_close(&file);

	//应用升级文件存在，将升级文件删除
	if (f_unlink("/update.bin") != FR_OK)
	{
		return -1;	//删除旧的资源文件失败
	}
#endif

#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)
	//record_delall(REC1BLK);
        for (int i = 0; i < DOWNLOADE_FILE_SIZE/(BLOCK_ERASE_SIZE*512);i++)
				{
					spi_flash_eraseblock(DOWNLOAD_FILE_START_SECT+i*BLOCK_ERASE_SIZE*512);
				}
#endif
	return 0;
}


/*******************************************************************************
* Function Name  : main
* Description    : Main program.
* Input          : None
* Input          : None
* Output         : 
* Output         : None
* Return         : None
*******************************************************************************/
int main(void)
{
//#ifdef DEBUG
//	debug();
//#endif

	/* System Clocks Configuration **********************************************/
	RCC_Configuration();

	/* NVIC Configuration *******************************************************/
	//	NVIC_SetVectorTable(NVIC_VectTab_FLASH+0x20000, 0);
	NVIC_SetVectorTable(NVIC_VectTab_FLASH, 0);

#if 0
	// 检查ROM是否被读保护，没有则执行读保护操作
	if( FLASH_GetReadOutProtectionStatus() != SET )
	{
		FLASH_Unlock();
		FLASH_ReadOutProtection(ENABLE);
	}
#endif
	/* Configure the NVIC Preemption Priority Bits */
	// 设置可抢占的优先级
	//NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);

	// 初始化升级指示的硬件资源
#ifdef LCD_VER
	Lcd_init();
#else
	LED_init();
#endif
	key_init();
	platform_misc_port_init();
	spi_flash_init();

	if ((if_feed_key_preesed() == 1)&&(hw_platform_USBcable_Insert_Detect() == 1))
	{
		goto sys_fail;//进入U盘升级模式
	}

recheck:
	// ==========================================================================================================
	// 1,初始化存储区（SD卡or SPI Flash），检查升级应用是否有效
	if( check_updatefile() == 0 )
	{
		if( update_app() != 0 )
		{
			goto sys_fail;
		}

		if (del_update_bin())
		{
			goto sys_fail;
		}
	}

	// ==========================================================================================================
	// 2,检查应用程序是否有效
	if( verify_app() == 0 )
	{
		// ==========================================================================================================
		// 3,运行应用
		RCC->CIR						= 0x00000000;				/* 关闭中断				*/
		JumpAddress						= *(volatile unsigned int *)(ApplicationAddress + 4);
		Jump_To_Application				= (pFunction)JumpAddress;

		__MSR_MSP(*(volatile unsigned int *) ApplicationAddress);
		Jump_To_Application();
	}

sys_fail:
#ifdef LED_VER
	LED_OFF();
#else
	Lcd_clear(1);
	Lcd_TextOut(0,12,"FW Upgrade");
#endif

	// ==========================================================================================================
	// 4,显示错误信息
	{
		//int			i;
#if(USB_DEVICE_CONFIG & _USE_USB_MASS_STOARGE_DEVICE)
        g_mass_storage_device_type = MASSTORAGE_DEVICE_TYPE_SPI_FLASH;
		usb_device_init(USB_MASSSTORAGE);
#endif
#if(USB_DEVICE_CONFIG & _USE_USB_PRINTER_HID_COMP_DEVICE)
		usb_device_init(USB_PRINTER_HID_COMP);
#endif
		//USB_Cable_Config(1);
		while(if_feed_key_preesed())
		{
#ifdef LED_VER
			LED_blink(1,200);
#endif
		}
		while(1)
		{
#ifdef LED_VER
			LED_blink(1,200);
#endif			
			if (if_feed_key_preesed())
			{
				//检测到进纸键被按下
				goto recheck;
			}
		}
	}
}

#ifdef  DEBUG
/*******************************************************************************
* Function Name  : assert_failed
* Description    : Reports the name of the source file and the source line number
*                  where the assert_param error has occurred.n
* Input          : - file: pointer to the source file name
*                  - line: assert_param error line source number
* Output         : None
* Return         : None
*******************************************************************************/
void assert_failed(u8* file, u32 line)
{ 
	/* User can add his own implementation to report the file name and line number,
	ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

	/* Infinite loop */
	while (1)
	{
	}
}
#endif

/*******************************************************************************
* Function Name  : RCC_Configuration
* Description    : Configures the different system clocks.
* Input          : None
* Output         : None
* Return         : None
*******************************************************************************/
static void RCC_Configuration(void)
{   
	vu32 i=0;
	/* RCC system reset(for debug purpose) */
	RCC_DeInit();

	/* Enable HSE							*/
	RCC_HSEConfig(RCC_HSE_ON);
	// 这里要做延时，才能兼容某些比较差的晶体，以便顺利起震	
	for(i=0; i<200000; i++);

	/* Wait till HSE is ready			*/
	if(RCC_WaitForHSEStartUp() == SUCCESS)
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

		/* PLLCLK = 12MHz * 6 = 72 MHz	*/
		RCC_PLLConfig(RCC_PLLSource_HSE_Div1, RCC_PLLMul_6);

		/* PLLCLK = 8MHz * 9 = 72 MHz	*/
		//RCC_PLLConfig(RCC_PLLSource_HSE_Div2, RCC_PLLMul_9);

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

#if 0
static unsigned long crc32_tab[] = 
{// polynomial $edb88320 

	0x00000000L, 0x77073096L, 0xee0e612cL, 0x990951baL, 0x076dc419L,
	0x706af48fL, 0xe963a535L, 0x9e6495a3L, 0x0edb8832L, 0x79dcb8a4L,
	0xe0d5e91eL, 0x97d2d988L, 0x09b64c2bL, 0x7eb17cbdL, 0xe7b82d07L,
	0x90bf1d91L, 0x1db71064L, 0x6ab020f2L, 0xf3b97148L, 0x84be41deL,
	0x1adad47dL, 0x6ddde4ebL, 0xf4d4b551L, 0x83d385c7L, 0x136c9856L,
	0x646ba8c0L, 0xfd62f97aL, 0x8a65c9ecL, 0x14015c4fL, 0x63066cd9L,
	0xfa0f3d63L, 0x8d080df5L, 0x3b6e20c8L, 0x4c69105eL, 0xd56041e4L,
	0xa2677172L, 0x3c03e4d1L, 0x4b04d447L, 0xd20d85fdL, 0xa50ab56bL,
	0x35b5a8faL, 0x42b2986cL, 0xdbbbc9d6L, 0xacbcf940L, 0x32d86ce3L,
	0x45df5c75L, 0xdcd60dcfL, 0xabd13d59L, 0x26d930acL, 0x51de003aL,
	0xc8d75180L, 0xbfd06116L, 0x21b4f4b5L, 0x56b3c423L, 0xcfba9599L,
	0xb8bda50fL, 0x2802b89eL, 0x5f058808L, 0xc60cd9b2L, 0xb10be924L,
	0x2f6f7c87L, 0x58684c11L, 0xc1611dabL, 0xb6662d3dL, 0x76dc4190L,
	0x01db7106L, 0x98d220bcL, 0xefd5102aL, 0x71b18589L, 0x06b6b51fL,
	0x9fbfe4a5L, 0xe8b8d433L, 0x7807c9a2L, 0x0f00f934L, 0x9609a88eL,
	0xe10e9818L, 0x7f6a0dbbL, 0x086d3d2dL, 0x91646c97L, 0xe6635c01L,
	0x6b6b51f4L, 0x1c6c6162L, 0x856530d8L, 0xf262004eL, 0x6c0695edL,
	0x1b01a57bL, 0x8208f4c1L, 0xf50fc457L, 0x65b0d9c6L, 0x12b7e950L,
	0x8bbeb8eaL, 0xfcb9887cL, 0x62dd1ddfL, 0x15da2d49L, 0x8cd37cf3L,
	0xfbd44c65L, 0x4db26158L, 0x3ab551ceL, 0xa3bc0074L, 0xd4bb30e2L,
	0x4adfa541L, 0x3dd895d7L, 0xa4d1c46dL, 0xd3d6f4fbL, 0x4369e96aL,
	0x346ed9fcL, 0xad678846L, 0xda60b8d0L, 0x44042d73L, 0x33031de5L,
	0xaa0a4c5fL, 0xdd0d7cc9L, 0x5005713cL, 0x270241aaL, 0xbe0b1010L,
	0xc90c2086L, 0x5768b525L, 0x206f85b3L, 0xb966d409L, 0xce61e49fL,
	0x5edef90eL, 0x29d9c998L, 0xb0d09822L, 0xc7d7a8b4L, 0x59b33d17L,
	0x2eb40d81L, 0xb7bd5c3bL, 0xc0ba6cadL, 0xedb88320L, 0x9abfb3b6L,
	0x03b6e20cL, 0x74b1d29aL, 0xead54739L, 0x9dd277afL, 0x04db2615L,
	0x73dc1683L, 0xe3630b12L, 0x94643b84L, 0x0d6d6a3eL, 0x7a6a5aa8L,
	0xe40ecf0bL, 0x9309ff9dL, 0x0a00ae27L, 0x7d079eb1L, 0xf00f9344L,
	0x8708a3d2L, 0x1e01f268L, 0x6906c2feL, 0xf762575dL, 0x806567cbL,
	0x196c3671L, 0x6e6b06e7L, 0xfed41b76L, 0x89d32be0L, 0x10da7a5aL,
	0x67dd4accL, 0xf9b9df6fL, 0x8ebeeff9L, 0x17b7be43L, 0x60b08ed5L,
	0xd6d6a3e8L, 0xa1d1937eL, 0x38d8c2c4L, 0x4fdff252L, 0xd1bb67f1L,
	0xa6bc5767L, 0x3fb506ddL, 0x48b2364bL, 0xd80d2bdaL, 0xaf0a1b4cL,
	0x36034af6L, 0x41047a60L, 0xdf60efc3L, 0xa867df55L, 0x316e8eefL,
	0x4669be79L, 0xcb61b38cL, 0xbc66831aL, 0x256fd2a0L, 0x5268e236L,
	0xcc0c7795L, 0xbb0b4703L, 0x220216b9L, 0x5505262fL, 0xc5ba3bbeL,
	0xb2bd0b28L, 0x2bb45a92L, 0x5cb36a04L, 0xc2d7ffa7L, 0xb5d0cf31L,
	0x2cd99e8bL, 0x5bdeae1dL, 0x9b64c2b0L, 0xec63f226L, 0x756aa39cL,
	0x026d930aL, 0x9c0906a9L, 0xeb0e363fL, 0x72076785L, 0x05005713L,
	0x95bf4a82L, 0xe2b87a14L, 0x7bb12baeL, 0x0cb61b38L, 0x92d28e9bL,
	0xe5d5be0dL, 0x7cdcefb7L, 0x0bdbdf21L, 0x86d3d2d4L, 0xf1d4e242L,
	0x68ddb3f8L, 0x1fda836eL, 0x81be16cdL, 0xf6b9265bL, 0x6fb077e1L,
	0x18b74777L, 0x88085ae6L, 0xff0f6a70L, 0x66063bcaL, 0x11010b5cL,
	0x8f659effL, 0xf862ae69L, 0x616bffd3L, 0x166ccf45L, 0xa00ae278L,
	0xd70dd2eeL, 0x4e048354L, 0x3903b3c2L, 0xa7672661L, 0xd06016f7L,
	0x4969474dL, 0x3e6e77dbL, 0xaed16a4aL, 0xd9d65adcL, 0x40df0b66L,
	0x37d83bf0L, 0xa9bcae53L, 0xdebb9ec5L, 0x47b2cf7fL, 0x30b5ffe9L,
	0xbdbdf21cL, 0xcabac28aL, 0x53b39330L, 0x24b4a3a6L, 0xbad03605L,
	0xcdd70693L, 0x54de5729L, 0x23d967bfL, 0xb3667a2eL, 0xc4614ab8L,
	0x5d681b02L, 0x2a6f2b94L, 0xb40bbe37L, 0xc30c8ea1L, 0x5a05df1bL,
	0x2d02ef8dL
};

/* Return a 32-bit CRC of the contents of the buffer. */

unsigned long crc32 (unsigned long crc, unsigned char *buf, int len)
{
	unsigned char *end;

	crc = ~crc;
	for (end = buf + len; buf < end; ++buf)
		crc = crc32_tab[(crc ^ *buf) & 0xff] ^ (crc >> 8);
	return ~crc;
}

#endif




/******************* (C) COPYRIGHT 2008 STMicroelectronics *****END OF FILE****/