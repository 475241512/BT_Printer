/**
* @file spi_flash_config.h
* @brief BT_printer项目中SPI FLASH的使用规划及相关配置
*        
* @version V0.0.1
* @author kent.zhou
* @date 2015年12月01日
* @note
*     
*     具体规划如下：
*	  8M SPI FLASH： 从SPI FLASH的第一个Sector开始的1M空间，由recmod驱动去管理，用来保存掉电需要保存的参数
*					 从SPI FLASH的第1M偏移开始的5M空间，由spi flash最底层的驱动去管理，用来保存字库数据，需要加快读取速度，所以用最底层的驱动去管理
*					 从SPI FLASH的第6M偏移开始的2M空间，由FAT文件系统去管理，负责接收从U盘接口传输下来的数据	

*/

#ifndef _SPI_FLASH_CONFIG_H_
#define _SPI_FLASH_CONFIG_H_

#define			BLOCK_ERASE_SIZE		128				//Sector		64K Block Erase

#define			FLASH_SIZE			(8*1024*1024)			//8M

#define PARAM_FLASH_SIZE		((1024-256-64-512)*1024)					//1M 保存参数
#define BT_DATA_CACHE_SIZE		512*1024						//512K用来保存蓝牙接收到的数据			
#define DOWNLOADE_FILE_SIZE		256*1024					//256K用来保存下载下来的文件			
#define RES_FLASH_SIZE			(5*1024*1024)				//5M 字库等资源文件
#define FAT_FLASH_SIZE			((2*1024+64)*1024)				//2M  FAT文件系统	

#define PARAM_START_SECT		0									//Param start sector,
#define BT_CACHE_START_SECT		PARAM_FLASH_SIZE					//BT Cache start sector,
#define DOWNLOAD_FILE_START_SECT		(BT_CACHE_START_SECT+BT_DATA_CACHE_SIZE)			//DownLoad File start sector,
#define RES_START_SECT			(DOWNLOAD_FILE_START_SECT+DOWNLOADE_FILE_SIZE)		//Resource start sector
#define FAT_START_SECT			(RES_START_SECT+RES_FLASH_SIZE)


#endif