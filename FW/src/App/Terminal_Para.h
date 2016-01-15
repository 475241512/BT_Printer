/**
*  @file 	Terminal_Para.h
*  @brief  	定义了终端需要保存在NVM区域的参数的数据结构
*  @note	
*/

#ifndef _TERMINAL_PARA_H_
#define _TERMINAL_PARA_H_
#include "hw_platform.h"
#include "BT816.h"


/**
*@brief 定义存储在SPI FLASH内部终端参数的结构类型
*@ note	
*/
#pragma pack(1)

typedef struct  {
	unsigned int			checkvalue;					//4字节		0	B	此份结构的校验值 crc32			
	unsigned int			line_after_mark;			//4字节		4	B
	unsigned int 			max_mark_length;			//4字节		8	B
	unsigned char 			character_code_page;		//1字节		12	B
	unsigned char			bt_pin[MAX_BT_CHANNEL][5];	//保存的蓝牙模块当前的PIN
	unsigned char			bt_mac[MAX_BT_CHANNEL][13];	//保存的蓝牙模块MAC地址的后4位
	unsigned char			bt_version[17];				//蓝牙模块的版本号
	unsigned int			bt1_flash_cache_write_offset;
	unsigned int			bt1_flash_cache_read_offset;
	unsigned char			rfu[4];						//47字节	13  B
	unsigned char			struct_ver;					//1字节		60	B	参数版本号，标识此结构的版本
	unsigned char			endtag[3];					//0x55,0xAA,0x5A  61      一共64字节
} TTerminalPara;
#pragma pack()

extern TTerminalPara		g_param;				//终端参数

int ReadTerminalPara(void);
int SaveTerminalPara(void);
int DefaultTerminalPara(void);
#endif
