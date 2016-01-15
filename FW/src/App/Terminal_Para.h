/**
*  @file 	Terminal_Para.h
*  @brief  	�������ն���Ҫ������NVM����Ĳ��������ݽṹ
*  @note	
*/

#ifndef _TERMINAL_PARA_H_
#define _TERMINAL_PARA_H_
#include "hw_platform.h"
#include "BT816.h"


/**
*@brief ����洢��SPI FLASH�ڲ��ն˲����Ľṹ����
*@ note	
*/
#pragma pack(1)

typedef struct  {
	unsigned int			checkvalue;					//4�ֽ�		0	B	�˷ݽṹ��У��ֵ crc32			
	unsigned int			line_after_mark;			//4�ֽ�		4	B
	unsigned int 			max_mark_length;			//4�ֽ�		8	B
	unsigned char 			character_code_page;		//1�ֽ�		12	B
	unsigned char			bt_pin[MAX_BT_CHANNEL][5];	//���������ģ�鵱ǰ��PIN
	unsigned char			bt_mac[MAX_BT_CHANNEL][13];	//���������ģ��MAC��ַ�ĺ�4λ
	unsigned char			bt_version[17];				//����ģ��İ汾��
	unsigned int			bt1_flash_cache_write_offset;
	unsigned int			bt1_flash_cache_read_offset;
	unsigned char			rfu[4];						//47�ֽ�	13  B
	unsigned char			struct_ver;					//1�ֽ�		60	B	�����汾�ţ���ʶ�˽ṹ�İ汾
	unsigned char			endtag[3];					//0x55,0xAA,0x5A  61      һ��64�ֽ�
} TTerminalPara;
#pragma pack()

extern TTerminalPara		g_param;				//�ն˲���

int ReadTerminalPara(void);
int SaveTerminalPara(void);
int DefaultTerminalPara(void);
#endif
