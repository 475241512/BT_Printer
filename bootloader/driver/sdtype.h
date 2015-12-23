#ifndef _SD_TYPE_H
#define _SD_TYPE_H


//typedef enum
//{
//
//} SD_Error;

/* Exported types ------------------------------------------------------------*/
typedef enum
{
	/* SDIO specific error defines */
	SD_CMD_CRC_FAIL                    = (1), /* Command response received (but CRC check failed) */
	SD_DATA_CRC_FAIL                   = (2), /* Data bock sent/received (CRC check Failed) */
	SD_CMD_RSP_TIMEOUT                 = (3), /* Command response timeout */
	SD_DATA_TIMEOUT                    = (4), /* Data time out */
	SD_TX_UNDERRUN                     = (5), /* Transmit FIFO under-run */
	SD_RX_OVERRUN                      = (6), /* Receive FIFO over-run */
	SD_START_BIT_ERR                   = (7), /* Start bit not detected on all data signals in widE bus mode */
	SD_CMD_OUT_OF_RANGE                = (8), /* CMD's argument was out of range.*/
	SD_ADDR_MISALIGNED                 = (9), /* Misaligned address */
	SD_BLOCK_LEN_ERR                   = (10), /* Transferred block length is not allowed for the card or the number of transferred bytes does not match the block length */
	SD_ERASE_SEQ_ERR                   = (11), /* An error in the sequence of erase command occurs.*/
	SD_BAD_ERASE_PARAM                 = (12), /* An Invalid selection for erase groups */
	SD_WRITE_PROT_VIOLATION            = (13), /* Attempt to program a write protect block */
	SD_LOCK_UNLOCK_FAILED              = (14), /* Sequence or password error has been detected in unlock command or if there was an attempt to access a locked card */
	SD_COM_CRC_FAILED                  = (15), /* CRC check of the previous command failed */
	SD_ILLEGAL_CMD                     = (16), /* Command is not legal for the card state */
	SD_CARD_ECC_FAILED                 = (17), /* Card internal ECC was applied but failed to correct the data */
	SD_CC_ERROR                        = (18), /* Internal card controller error */
	SD_GENERAL_UNKNOWN_ERROR           = (19), /* General or Unknown error */
	SD_STREAM_READ_UNDERRUN            = (20), /* The card could not sustain data transfer in stream read operation. */
	SD_STREAM_WRITE_OVERRUN            = (21), /* The card could not sustain data programming in stream mode */
	SD_CID_CSD_OVERWRITE               = (22), /* CID/CSD overwrite error */
	SD_WP_ERASE_SKIP                   = (23), /* only partial address space was erased */
	SD_CARD_ECC_DISABLED               = (24), /* Command has been executed without using internal ECC */
	SD_ERASE_RESET                     = (25), /* Erase sequence was cleared before executing because an out of erase sequence command was received */
	SD_AKE_SEQ_ERROR                   = (26), /* Error in sequence of authentication. */
	SD_INVALID_VOLTRANGE               = (27),
	SD_ADDR_OUT_OF_RANGE               = (28),
	SD_SWITCH_ERROR                    = (29),
	SD_SDIO_DISABLED                   = (30),
	SD_SDIO_FUNCTION_BUSY              = (31),
	SD_SDIO_FUNCTION_FAILED            = (32),
	SD_SDIO_UNKNOWN_FUNCTION           = (33),

	/* Standard error defines */
	SD_INTERNAL_ERROR, 
	SD_NOT_CONFIGURED,
	SD_REQUEST_PENDING, 
	SD_REQUEST_NOT_APPLICABLE, 
	SD_INVALID_PARAMETER,  
	SD_UNSUPPORTED_FEATURE,  
	SD_UNSUPPORTED_HW,  
	SD_ERROR,  
	SD_OK,  

	/**
	* @brief  SD reponses and error flags
	*/
	SD_RESPONSE_NO_ERROR      = (0x00),
	SD_IN_IDLE_STATE          = (0x01),
	//SD_ERASE_RESET            = (0x02),
	SD_ILLEGAL_COMMAND        = (0x04),
	SD_COM_CRC_ERROR          = (0x08),
	SD_ERASE_SEQUENCE_ERROR   = (0x10),
	SD_ADDRESS_ERROR          = (0x20),
	SD_PARAMETER_ERROR        = (0x40),
	SD_BUSY					  = (0x41),
	SD_RESPONSE_FAILURE       = (0xFF),

	/**
	* @brief  Data response error
	*/
	SD_DATA_OK                = (0x05),
	SD_DATA_CRC_ERROR         = (0x0B),
	SD_DATA_WRITE_ERROR       = (0x0D),
	SD_DATA_OTHER_ERROR       = (0xFF),

} SD_Error;

/** 
* @brief  Card Specific Data: CSD Register   
*/ 
typedef struct
{
	volatile unsigned char  CSDStruct;            /*!< CSD structure */
	volatile unsigned char  SysSpecVersion;       /*!< System specification version */
	volatile unsigned char  Reserved1;            /*!< Reserved */
	volatile unsigned char  TAAC;                 /*!< Data read access-time 1 */
	volatile unsigned char  NSAC;                 /*!< Data read access-time 2 in CLK cycles */
	volatile unsigned char  MaxBusClkFrec;        /*!< Max. bus clock frequency */
	volatile unsigned short CardComdClasses;      /*!< Card command classes */
	volatile unsigned char  RdBlockLen;           /*!< Max. read data block length */
	volatile unsigned char  PartBlockRead;        /*!< Partial blocks for read allowed */
	volatile unsigned char  WrBlockMisalign;      /*!< Write block misalignment */
	volatile unsigned char  RdBlockMisalign;      /*!< Read block misalignment */
	volatile unsigned char  DSRImpl;              /*!< DSR implemented */
	volatile unsigned char  Reserved2;            /*!< Reserved */
	volatile unsigned int DeviceSize;           /*!< Device Size */
	volatile unsigned char  MaxRdCurrentVDDMin;   /*!< Max. read current @ VDD min */
	volatile unsigned char  MaxRdCurrentVDDMax;   /*!< Max. read current @ VDD max */
	volatile unsigned char  MaxWrCurrentVDDMin;   /*!< Max. write current @ VDD min */
	volatile unsigned char  MaxWrCurrentVDDMax;   /*!< Max. write current @ VDD max */
	volatile unsigned char  DeviceSizeMul;        /*!< Device size multiplier */
	volatile unsigned char  EraseGrSize;          /*!< Erase group size */
	volatile unsigned char  EraseGrMul;           /*!< Erase group size multiplier */
	volatile unsigned char  WrProtectGrSize;      /*!< Write protect group size */
	volatile unsigned char  WrProtectGrEnable;    /*!< Write protect group enable */
	volatile unsigned char  ManDeflECC;           /*!< Manufacturer default ECC */
	volatile unsigned char  WrSpeedFact;          /*!< Write speed factor */
	volatile unsigned char  MaxWrBlockLen;        /*!< Max. write data block length */
	volatile unsigned char  WriteBlockPaPartial;  /*!< Partial blocks for write allowed */
	volatile unsigned char  Reserved3;            /*!< Reserded */
	volatile unsigned char  ContentProtectAppli;  /*!< Content protection application */
	volatile unsigned char  FileFormatGrouop;     /*!< File format group */
	volatile unsigned char  CopyFlag;             /*!< Copy flag (OTP) */
	volatile unsigned char  PermWrProtect;        /*!< Permanent write protection */
	volatile unsigned char  TempWrProtect;        /*!< Temporary write protection */
	volatile unsigned char  FileFormat;           /*!< File Format */
	volatile unsigned char  ECC;                  /*!< ECC code */
	volatile unsigned char  CSD_CRC;              /*!< CSD CRC */
	volatile unsigned char  Reserved4;            /*!< always 1*/
} SD_CSD;


/** 
* @brief  Card Identification Data: CID Register   
*/
typedef struct
{
	volatile unsigned char  ManufacturerID;       /*!< ManufacturerID */
	volatile unsigned short OEM_AppliID;          /*!< OEM/Application ID */
	volatile unsigned int ProdName1;            /*!< Product Name part1 */
	volatile unsigned char  ProdName2;            /*!< Product Name part2*/
	volatile unsigned char  ProdRev;              /*!< Product Revision */
	volatile unsigned int ProdSN;               /*!< Product Serial Number */
	volatile unsigned char  Reserved1;            /*!< Reserved1 */
	volatile unsigned short ManufactDate;         /*!< Manufacturing Date */
	volatile unsigned char  CID_CRC;              /*!< CID CRC */
	volatile unsigned char  Reserved2;            /*!< always 1 */
} SD_CID;

/** 
* @brief SD Card information 
*/
typedef struct
{
	SD_CSD SD_csd;
	SD_CID SD_cid;
	unsigned int CardCapacity; /* Card Capacity µ¥Î»: K */  
	unsigned int CardBlockSize; /* Card Block Size */
	unsigned short RCA;
	unsigned char CardType;
} SD_CardInfo;

#endif //_SD_TYPE_H