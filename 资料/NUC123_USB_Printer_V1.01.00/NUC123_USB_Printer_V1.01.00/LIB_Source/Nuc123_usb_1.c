//=======================================================================================
//  Copyright Nuvoton(Shang-Hai) Technology Corp. All rights reserved. 2014 
//=======================================================================================
#define   USB_GLOBAL
#include  "NUC123Series.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h" 

extern  void  USB_Tx_OVER(void) ;                //相当于发送结束中断函数
extern  void  USB_Rx_Data(void) ;                //相当于接收到数据中断函数
extern  void  PTR_Data_Receive(void) ;              

#define  GET_STATUS                0x00                    //USB协议定义的11种标准请求
#define  CLEAR_FEATURE             0x01
#define  SET_FEATURE               0x03
#define  SET_ADDRESS               0x05
#define  GET_DESCRIPTOR            0x06
#define  SET_DESCRIPTOR            0x07
#define  GET_CONFIGURATION         0x08
#define  SET_CONFIGURATION         0x09
#define  GET_INTERFACE             0x0A
#define  SET_INTERFACE             0x0B
#define  SYNC_FRAME                0x0C

#define  DESC_DEVICE               0x01                    //USB协议定义的几种描述符请求
#define  DESC_CONFIG               0x02
#define  DESC_STRING               0x03
#define  DESC_INTERFACE            0x04
#define  DESC_ENDPOINT             0x05
#define  DESC_QUALIFIER            0x06
#define  DESC_OTHERSPEED           0x07

#define  LEN_DEVICE                18                      //USB协议定义的设备描述符长度
#define  LEN_CONFIG                9                       //USB协议定义的配置描述符长度
#define	 LEN_INTERFACE             9                       //USB协议定义的接口描述符长度
#define	 LEN_ENDPOINT              7                       //USB协议定义的端点描述符长度

#define  EP_BULK                   0x02                    //此四个是USB协议中定义的端点属性值
#define  EP_INT                    0x03
#define  EP_INPUT                  0x80
#define  EP_OUTPUT                 0x00
 
#define  FLAG_SET_LINE_CODE        0x0200                  // CDC Class 
#define  DESC_HID                  0x21                    // USB HID Descriptor Type 
#define  DESC_HID_RPT              0x22                    // USB HID Descriptor Type 

#define  SEND_ENCAPSULATED_COMMAND 0x00                    // CDC Class Specific Request 
#define  GET_ENCAPSULATED_RESPONSE 0x01
#define  SET_COMM_FEATURE          0x02
#define  GET_COMM_FEATURE          0x03
#define  CLEAR_COMM_FEATURE        0x04
#define  SET_LINE_CODE             0x20
#define  GET_LINE_CODE             0x21
#define  SET_CONTROL_LINE_STATE    0x22
#define  SEND_BREAK                0x23
                                                           //用于写端点配置 CFG 的位定义
#define  USB_OUT_ENDP              0x20                    //输出端点
#define  USB_ISOCH_OUT_ENDP        0x30                    //等时输出端点
#define  USB_IN_ENDP               0x40                    //输入端点     
#define  USB_ISOCH_IN_ENDP         0x50                    //等时输入端点

#define  USB_PACKET_SIZE_MAX       64                      // 最大USB包大小  
#define  USB_CFG_DSQ_SYNC_DATA1    0x0080                  // 数据序列 DATA1

#define  BULK_IN_EP_NUM            1                       //用到的三个端点地址
#define  BULK_OUT_EP_NUM           2
#define  INT_IN_EP_NUM             3 

#ifdef PRINTER_USB_VCOM    ////=========================================// 虚拟串口描述符

#define  USB_VID		           0x0416
#define  USB_PID		           0x5011
const uint8_t USB_DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength              */
	DESC_DEVICE,	/* bDescriptorType      */
	0x00, 0x02,		/* bcdUSB               */                 // USB版本
	0x02,			/* bDeviceClass         */                     // 设备类
	0x00,			/* bDeviceSubClass      */
	0x00,			/* bDeviceProtocol      */
	USB_PACKET_SIZE_MAX,	/* bMaxPacketSize0 */              // 端点0最大包
	/* idVendor */
	USB_VID & 0x00FF,                                        // 厂商编号
	(USB_VID & 0xFF00) >> 8,
	/* idProduct */
	USB_PID & 0x00FF,                                        // 产品编号
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x03,		/* bcdDevice            */                 // 设备编号
	0x01,			/* iManufacture         */                     // 厂商串索引
	0x02,			/* iProduct             */                     // 产品串索引
	0x00,			/* iSerialNumber        */                     // 序列号串索引
	0x01			/* bNumConfigurations   */
};
const uint8_t USB_ConfigDescriptor[] =
{
	LEN_CONFIG,		/* bLength              */
	DESC_CONFIG,	/* bDescriptorType      */
	0x43, 0x00,		/* wTotalLength         */                 //配置描述符总长度
	0x02,			/* bNumInterfaces       */
	0x01,			/* bConfigurationValue  */
	0x00,			/* iConfiguration       */
	0xC0,			/* bmAttributes         */
	0x32,			/* MaxPower             */

	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x00,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x01,			/* bNumEndpoints        */
	0x02,			/* bInterfaceClass      */                     // CDC control class
	0x02,			/* bInterfaceSubClass   */
	0x01,			/* bInterfaceProtocol   */
	0x01,			/* iInterface           */

	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x00,           /* Header functional descriptor subtype */
    0x10, 0x01,     /* Communication device compliant to the communication spec. ver. 1.10 */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x01,           /* Call management functional descriptor */
    0x00,           /* BIT0: Whether device handle call management itself. */
                    /* BIT1: Whether device can send/receive call management information over a Data Class Interface 0 */
    0x01,           /* Interface number of data class interface optionally used for call management */

	/* Communication Class Specified INTERFACE descriptor */
    0x04,           /* Size of the descriptor, in bytes */
    0x24,           /* CS_INTERFACE descriptor type */
    0x02,           /* Abstract control management funcational descriptor subtype */
    0x00,           /* bmCapabilities       */
    
	/* Communication Class Specified INTERFACE descriptor */
    0x05,           /* bLength              */
    0x24,           /* bDescriptorType: CS_INTERFACE descriptor type */
    0x06,           /* bDescriptorSubType   */
    0x00,           /* bMasterInterface     */
    0x01,           /* bSlaveInterface0     */
    
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | INT_IN_EP_NUM),     /* bEndpointAddress */   //端点地址:3,中断IN
	EP_INT,		                    /* bmAttributes     */
	USB_PACKET_SIZE_MAX, 0x00,	    /* wMaxPacketSize   */
	0x01,	                        /* bInterval        */     //间隔 1ms
			
	/* INTERFACE descriptor */
	LEN_INTERFACE,	/* bLength              */
	DESC_INTERFACE,	/* bDescriptorType      */
	0x01,			/* bInterfaceNumber     */
	0x00,			/* bAlternateSetting    */
	0x02,			/* bNumEndpoints        */
	0x0A,			/* bInterfaceClass      */                     // CDC data
	0x00,			/* bInterfaceSubClass   */
	0x00,			/* bInterfaceProtocol   */
	0x01,			/* iInterface           */
			
	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_INPUT | BULK_IN_EP_NUM),	/* bEndpointAddress */     //端点地址:1, Bulk in
	EP_BULK,		                /* bmAttributes     */
	USB_PACKET_SIZE_MAX, 0x00,	    /* wMaxPacketSize   */
	0x00,			                /* bInterval        */

	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_OUTPUT | BULK_OUT_EP_NUM),	/* bEndpointAddress */   //端点地址:2, Bulk out
	EP_BULK,		                /* bmAttributes     */
	USB_PACKET_SIZE_MAX, 0x00,     /* wMaxPacketSize   */
	0x00,			                /* bInterval        */
};
const uint8_t USB_ProductStringDesc[] =
{
	32,             /* bLength          */
	DESC_STRING,    /* bDescriptorType  */
	'U', 0, 
  'S', 0,	
  'B', 0,	
  ' ', 0,	
  'V', 0,	
  'i', 0,	
  'r', 0,	
  't', 0,	
  'u', 0,	
  'a', 0,
	'l', 0,	
	' ', 0,	
  'C', 0,	
  'O', 0,	
  'M', 0
};

#else     ////=============================================// 打印机描述符 ==============
#define  USB_VID		           0x0416
#define  USB_PID		           0xAABB
const uint8_t USB_DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength              */
	DESC_DEVICE,	/* bDescriptorType      */
	0x10, 0x01,		/* bcdUSB               */
	0x00,			/* bDeviceClass         */
	0x00,			/* bDeviceSubClass      */
	0x00,			/* bDeviceProtocol      */
	USB_PACKET_SIZE_MAX,	/* bMaxPacketSize0 */
	/* idVendor */
	USB_VID & 0x00FF,
	(USB_VID & 0xFF00) >> 8,
	/* idProduct */
	USB_PID & 0x00FF,
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x00,		/* bcdDevice            */
	0x01,			/* iManufacture         */
	0x02,			/* iProduct             */
	0x00,			/* iSerialNumber        */
	0x01			/* bNumConfigurations   */              
};
const uint8_t USB_ConfigDescriptor[] =
{
	LEN_CONFIG,		// bLength
	DESC_CONFIG,	// bDescriptorType
	// wTotalLength
	0x4E,	0x00,
	0x01,			// bNumInterfaces
	0x01,			// bConfigurationValue
	0x00,			// iConfiguration
  0xA0,
  50,       // MaxPower 2*50mA

	// I/F descr: Printer class
	LEN_INTERFACE,	// bLength
	DESC_INTERFACE,	// bDescriptorType
	0x00,			// bInterfaceNumber
	0x00,			// bAlternateSetting
	0x01,			// bNumEndpoints
	0x07,			// bInterfaceClass                             // Printer class
	0x01,			// bInterfaceSubClass
	0x01,			// bInterfaceProtocol
	0x00,			// iInterface                                  // string index

	// EP Descriptor: Bulk Out
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(BULK_OUT_EP_NUM | EP_OUTPUT),	// bEndpointAddress
	EP_BULK,		// bmAttributes
	// wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x00,		// bInterval

	// I/F descr: Printer class
	LEN_INTERFACE,	// bLength
	DESC_INTERFACE,	// bDescriptorType
	0x00,			// bInterfaceNumber
	0x01,			// bAlternateSetting
	0x02,			// bNumEndpoints
	0x07,			// bInterfaceClass
	0x01,			// bInterfaceSubClass
	0x02,			// bInterfaceProtocol
	0x00,			// iInterface

	// EP Descriptor: Bulk Out
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(BULK_OUT_EP_NUM | EP_OUTPUT),	// bEndpointAddress
	EP_BULK,		// bmAttributes
	// wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x00,		// bInterval

	// EP Descriptor: Bulk In
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(BULK_IN_EP_NUM | EP_INPUT),	// bEndpointAddress
	EP_BULK,		// bmAttributes
	// wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x00,		// bInterval

	// I/F descr: Printer class
	LEN_INTERFACE,	// bLength
	DESC_INTERFACE,	// bDescriptorType
	0x00,			// bInterfaceNumber
	0x02,			// bAlternateSetting
	0x03,			// bNumEndpoints
	0xFF,			// bInterfaceClass
	0x00,			// bInterfaceSubClass
	0xFF,			// bInterfaceProtocol
	0x00,			// iInterface

	// EP Descriptor: Bulk Out
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(BULK_OUT_EP_NUM | EP_OUTPUT),	// bEndpointAddress
	EP_BULK,		// bmAttributes
	// wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x00,		// bInterval

	// EP Descriptor: Bulk In
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(BULK_IN_EP_NUM | EP_INPUT),	// bEndpointAddress
	EP_BULK,		// bmAttributes
	// wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x00,		// bInterval

	// EP Descriptor: Interrupt In
	LEN_ENDPOINT,	// bLength
	DESC_ENDPOINT,	// bDescriptorType
	(INT_IN_EP_NUM | EP_INPUT),	// bEndpointAddress
	EP_INT,		// bmAttributes
	                                  // wMaxPacketSize
	USB_PACKET_SIZE_MAX & 0x00FF,
	(USB_PACKET_SIZE_MAX & 0xFF00) >> 8,
	0x01		// bInterval
};
const uint8_t USB_ProductStringDesc[] =
{
	54,
	DESC_STRING,
	  'N', 0,
    'u', 0,
    'v', 0,
    'o', 0,
    't', 0,
    'o', 0,
    'n', 0,
    ' ', 0,
    'U', 0,
    'S', 0,
    'B', 0,
    ' ', 0,
    'P', 0,
    'r', 0,
    'i', 0,
    'n', 0,
    't', 0,
    'e', 0,
    'r', 0,
    '_', 0,
    'N', 0,
    'U', 0,
    'C', 0,
    '1', 0,
    '2', 0,
    '3', 0,
};

#endif    ////===========================================================================

//如果不支持字符串请求,这些字符串就可以不定义,节省代码空间
const uint8_t USB_StringLang[4] =                          // 语言类
{
	4,				/* bLength */
	DESC_STRING,	/* bDescriptorType */
	0x09, 0x04                                  
};

const uint8_t USB_VendorStringDesc[] =
{
	16,
	DESC_STRING,
	'N', 0, 
  'u', 0, 
  'v', 0, 
  'o', 0, 
  't', 0, 
  'o', 0, 
  'n', 0
};


#define  Maximum(a,b)              (a)>(b) ? (a) : (b)
#define  Minimum(a,b)              (a)<(b) ? (a) : (b)

//这两个指针宏定义，注意要和初始化SRAM缓存的地址一致    
#define  pSetUp              ((uint8_t*)USB_SRAM)          //SETUP数据放在SRAM 0
#define  pEnd0Data           ((uint8_t*)(USB_SRAM+8))      //端点0/1数据放在SRAM 8

//// USB 初始化 =========================================================================
void USB_Init(void)
{
  uint32_t volatile temp  = 900 ;
 
  CLK->APBCLK  |=  CLK_APBCLK_USBD_EN_Msk ;                // Enable USB Clock
  SYS->IPRSTC2 |=  (1ul << 27) ;                           // Reset USB 
  SYS->IPRSTC2 &= ~(1ul << 27) ;

  USBD->ATTR |= USBD_ATTR_BYTEMODE ;
  USBD->DRVSE0 = 1 ;	
  while(temp--) ;   
  
  USBD->STBUFSEG  = 0 ;                                           //Setup的8个字节存放首址                           
  USBD->EP[0].BUFSEG = 8 ;				                                //数据首址
  USBD->EP[0].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP) ;    //端点地址:0
  USBD->EP[1].BUFSEG = 8 ;				                  
  USBD->EP[1].CFG    = (USBD_EPCFG_CLEARSTALL | USB_OUT_ENDP) ;   //端点地址都为0,且共用一个数据区

  USBD->EP[2].BUFSEG = 8+64 ;				                                            //数据首址
  USBD->EP[2].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP | BULK_IN_EP_NUM);  //端点地址:1,BULK_IN_EP_NUM
  USBD->EP[3].BUFSEG = 8+64+64 ;                                                //数据首址
  USBD->EP[3].CFG    = (USBD_EPCFG_CLEARSTALL | USB_OUT_ENDP | BULK_OUT_EP_NUM);//端点地址:2,BULK_OUT_EP_NUM
  USBD->EP[4].BUFSEG = 8+64+64+64 ;                                             //数据首址
  USBD->EP[4].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP | INT_IN_EP_NUM) ;  //端点地址:3,INT_IN_EP_NUM

  USBD->INTEN = USBD_WAKUP_IE_ENABLE | USBD_INT_FLDET | USBD_INT_USB | USBD_INT_BUS ;
  NVIC_SetPriority(USBD_IRQn, 2) ;                                   // 中断优先级为 2, 低于走纸电机中断
  NVIC->ISER[0] = 1 << USBD_IRQn ;                                   // 打开中断向量
  USBD->DRVSE0 = 0 ;
  Vcom_TxSize  = 0 ;                                                 //标示没有数据要发送

  PDMA2->SAR  = USBD->EP[3].BUFSEG ;                       // 源地址: EP[3]的 RAM
  PDMA2->CSR  = 0x00000025 +(1 << 19) ;                    // 8位,源固定,目的加,IP->MEM  
  PDMA2->CSR |=  2 ;                                       // 复位 PDMA0
}

void USB_CtrlOut_Ack(void)
{
  switch(gUsbCtrl.StateFlag){                                    
    case FLAG_SET_LINE_CODE :
//      if(gUsbInfo.UsbDeviceClass & USB_CLASS_CDC){
//        MultiByte_copy((void *)&gCdcInfo,(uint8_t*)(USBD_SRAM_BASE+USBD->BUFSEG0),USBD->MXPLD1);
//        USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1 ;
//        USBD->MXPLD0 = 0 ;                                 //状态阶段准备
//      }
      break ;

    case 0 :                       //状态阶段OUT_ACK数据上传无错,若出错主机会发起重传
    default :
      break ;
  }
  gUsbCtrl.StateFlag = 0;
}

// 端点0的Ctrl_IN数据,即控制传输的状态阶段 ==============================================
void USB_CtrlIn_Ack(void)
{
  uint32_t  Cnt ;

  switch (gUsbCtrl.StateFlag){
    case FLAG_EP0_DATA_IN :                                //上次数据事务满包
      if(gUsbCtrl.ReqLen){
        Cnt = 0 ;
        do{
          pEnd0Data[Cnt] = *gUsbCtrl.pDesc++ ;
          if(++Cnt >= USB_PACKET_SIZE_MAX) break ;
        }while(--gUsbCtrl.ReqLen) ;
        USBD->EP[0].MXPLD = Cnt ;
      }
      else{                                                // 数据已全部上传
        USBD->EP[0].MXPLD  = 0 ;
        USBD->EP[1].MXPLD  = 0 ;                           // 启动状态阶段
        gUsbCtrl.StateFlag = 0 ;                           // 状态结束后,不做处理 
      }	
      return ;
                                                           //以下是状态阶段的 IN_ACK 
    case FLAG_SET_ADDRESS :                                //配置地址过程结束
        USBD->FADDR = pSetUp[2];                           //这里省去了StatusFlag清零
    default:
        return;    
  }
}

//// 类请求功能, 为简化代码,这个功能可以不支持 ==========================================
#define  GET_DEVICE_ID             0x00
#define  GET_PORT_STATUS           0x01
#define  SOFT_RESET                0x02
uint16_t gVCOMStatus ;    
void USB_ClassRequest(void)
{
  switch(pSetUp[1]){
    case GET_LINE_CODE :
      pEnd0Data[0] = (char)115200 ;         pEnd0Data[1] = (char)(115200 >> 8) ; 
      pEnd0Data[2] = (char)(115200 >> 16) ; pEnd0Data[3] = (char)(115200 >> 24) ; 
      pEnd0Data[4] = 0 ;                                   // 1-stop
      pEnd0Data[5] = 0 ;                                   // No parity
      pEnd0Data[6] = 8 ;                                   // 8bit
      USBD->EP[0].CFG  |= 0x80 ;   
      USBD->EP[0].MXPLD = 7 ;                  
    
      USBD->EP[1].MXPLD = 0 ;                              // 状态阶段的OUT准备
      break ;
    
    case SET_LINE_CODE :                                  
      USBD->EP[1].MXPLD = 7 ;                              // Out波特率等数据丢掉 
    
      USBD->EP[0].CFG  |= 0x80;   
      USBD->EP[0].MXPLD = 0;                               // 状态阶段准备
      break ;
    
    case SET_CONTROL_LINE_STATE : 
      gVCOMStatus = ((uint16_t)pSetUp[3]) + pSetUp[2] ;  
    
      USBD->EP[0].CFG  |= 0x80;   
      USBD->EP[0].MXPLD = 0;                               // 状态阶段准备
      break ;
    
//     case GET_PORT_STATUS :                                 // In PtrStatus
//       USBD->EP[0].CFG  |= USB_CFG_DSQ_SYNC_DATA1 ;
//       USBD->EP[0].MXPLD = 0 ;                              //状态阶段准备
//       break ;
    default :
    USBD->EP[0].CFGP |= 2 ;                                //功能不支持,回STALL
    USBD->EP[1].CFGP |= 2 ;
  }
}

/// 几种描述符请求, 为简化代码, 取消了字符串请求功能 ====================================
void USB_GetDescriptor(void)
{
  uint32_t Cnt ;
  uint8_t *ptr ;

  gUsbCtrl.ReqLen = pSetUp[6] + ((uint32_t)pSetUp[7]<<8);  //传输长度
  USBD->EP[0].CFG |= 0x80;                                 //之前是SETUP,此位不会Toggle
  gUsbCtrl.StateFlag = 0 ;             //先假定函数结束后是状态阶段,且状态过后什么也不做
            
  switch(pSetUp[3]){                                       //这个字节决定是什么描述符
    case DESC_DEVICE:                                      //设备描述符, 18字节
      ptr = (uint8_t*)USB_DeviceDescriptor ;
      Cnt = 0 ;
      do{
        pEnd0Data[Cnt] = *ptr++ ;
      }while(++Cnt <= 18);                                 //设备描述符18个字节
      USBD->EP[0].MXPLD = 18 ;
      break;
	 
    case DESC_CONFIG:                            //配置描述符, 长度不定, 此67个字节
      gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, (USB_ConfigDescriptor[3]<<8)|USB_ConfigDescriptor[2]);
      ptr = (uint8_t*)USB_ConfigDescriptor ;
      Cnt = 0 ;
      if(gUsbCtrl.ReqLen  >= USB_PACKET_SIZE_MAX){	
        do{pEnd0Data[Cnt]  = *ptr++ ;}while(++Cnt <= USB_PACKET_SIZE_MAX) ;
        USBD->EP[0].MXPLD  = USB_PACKET_SIZE_MAX;
        gUsbCtrl.pDesc     = (uint8_t *)USB_ConfigDescriptor + USB_PACKET_SIZE_MAX ;
        gUsbCtrl.ReqLen   -= USB_PACKET_SIZE_MAX;
        gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
      }
      else{
        while(Cnt < gUsbCtrl.ReqLen){ pEnd0Data[Cnt] = *ptr++ ; ++Cnt;}
        USBD->EP[0].MXPLD  = Cnt;
        USBD->EP[1].MXPLD  = 0 ;                           // 状态阶段准备
      }
      break;

   case DESC_STRING:                                       //串描述符
     Cnt = 0 ;
     if(pSetUp[2] == 0){                                   // 语言类
       gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_StringLang[0]);
       ptr = (uint8_t*)USB_StringLang ;
       do{
         pEnd0Data[Cnt] = *ptr++ ;
       }while(++Cnt <= gUsbCtrl.ReqLen);
       USBD->EP[0].MXPLD = gUsbCtrl.ReqLen;
       break;
     }
//      else if(pSetUp[2] == 1){                              // 供应商
//        gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_VendorStringDesc[0]);
//        ptr = (uint8_t*)USB_VendorStringDesc ;
//        Cnt = 0 ;
//        if(gUsbCtrl.ReqLen  >= USB_PACKET_SIZE_MAX){	
//          do{pEnd0Data[Cnt]  = *ptr++ ;}while(++Cnt <= USB_PACKET_SIZE_MAX) ;
//          USBD->EP[0].MXPLD  = USB_PACKET_SIZE_MAX;
//          gUsbCtrl.pDesc     = (uint8_t *)USB_ConfigDescriptor + USB_PACKET_SIZE_MAX ;
//          gUsbCtrl.ReqLen   -= USB_PACKET_SIZE_MAX;
//          gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
//        }
//        else{
//          while(Cnt < gUsbCtrl.ReqLen){ pEnd0Data[Cnt] = *ptr++ ; ++Cnt;}
//          USBD->EP[0].MXPLD = Cnt;
//        }
//        break;
//      }
     else if(pSetUp[2] == 2){                              // 设备名
       gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_ProductStringDesc[0]);
       ptr = (uint8_t*)USB_ProductStringDesc ;
       if(gUsbCtrl.ReqLen  >= USB_PACKET_SIZE_MAX){	
        do{pEnd0Data[Cnt]  = *ptr++ ;}while(++Cnt <= USB_PACKET_SIZE_MAX) ;
        USBD->EP[0].MXPLD  = USB_PACKET_SIZE_MAX;
        gUsbCtrl.pDesc     = (uint8_t *)USB_ConfigDescriptor + USB_PACKET_SIZE_MAX ;
        gUsbCtrl.ReqLen   -= USB_PACKET_SIZE_MAX;
        gUsbCtrl.StateFlag = FLAG_EP0_DATA_IN;
      }
      else{
        while(Cnt < gUsbCtrl.ReqLen){ pEnd0Data[Cnt] = *ptr++ ; ++Cnt;}
        USBD->EP[0].MXPLD = Cnt;
      }
       break;
     }
///	 	
  case DESC_HID_RPT:
    default:
      USBD->EP[0].CFGP |= 2 ;                              //功能不支持
      USBD->EP[1].CFGP |= 2 ;
      return ;
	}
    USBD->EP[1].MXPLD = 0;              //状态阶段准备, 这些都是IN事务, 先做状态阶段的OUT准备
	return ;
}

void EP3_Rx_Ready(void)
{
  USBD->EP[3].MXPLD = USB_PACKET_SIZE_MAX ;                //端点3 Rx准备接收下一包数据
  PRT_Flag |= PRINT_RECEIVING ;
}
//// 11种标准请求,这里只处理用到的几个,可简化代码 =======================================
void USB_StandardRequest(void)
{
  gUsbCtrl.StateFlag = 0 ;             //先假定函数结束后是状态阶段,且状态过后什么也不做

  switch (pSetUp[1]){                                      //请求描述符
    case GET_DESCRIPTOR:
      USB_GetDescriptor() ; 
      return  ;

    case SET_ADDRESS:                                      //设定地址
      gUsbCtrl.StateFlag = FLAG_SET_ADDRESS;               //无数据阶段
      USBD->EP[0].CFG |= USB_CFG_DSQ_SYNC_DATA1 ;    
      USBD->EP[0].MXPLD = 0;                               //状态阶段准备
      return; 
 
    case SET_CONFIGURATION:                                //设定配置值
      gUsbCtrl.ConfigState = pSetUp[2] ;                   //保存配置值
      USBD->EP[2].MXPLD = 0;                               //端点2 Tx数据0个
      EP3_Rx_Ready() ;
      USBD->EP[0].CFG  |= USB_CFG_DSQ_SYNC_DATA1 ;   
      USBD->EP[0].MXPLD = 0 ;                              //状态阶段准备
      return ; 
 
//    case CLEAR_FEATURE:
//    case SET_FEATURE:
//      if((pSetUp[0] == 2) && (USB->RAM[2] == FEATURE_ENDPOINT_HALT)){
//        uint32_t u32CfgAddr;
//        uint32_t u32Cfg;
//        uint32_t i;
//
//        u32Cfg = pSetUp[4] & 0xF;
//        for (i = 0; i < gTotalEnpointCount+2; i++) {
//          if(gUsbCtrl.EPInfo[i] == u32Cfg){
//            u32Cfg = i;
//            break;
//          }
//        }
//        u32CfgAddr = (uint32_t)(u32Cfg << 4) + (uint32_t)0x40060028; /* USBD_CFG0 */
//        u32Cfg = *((__IO uint32_t *) (u32CfgAddr));
//
//        if(pSetUp[1] == CLEAR_FEATURE)  u32Cfg &= ~USB_CFG_SSTALL;
//        else  u32Cfg |= USB_CFG_SSTALL;
//        *((__IO uint32_t *) (u32CfgAddr)) = u32Cfg;
//      }
//      else  return ; 
//				
//      USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
//      USBD->MXPLD0 = 0;
//      return ; 
  
//    case GET_CONFIGURATION:
//      *ptr = gUsbCtrl.ConfigState ;                        //就发送一个字节:配置值
//      USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1 ;
//      USBD->MXPLD0  = 1 ;                                  //数据事务准备
//      USBD->MXPLD1  = 0 ;                                  //状态阶段准备
//      return ; 
 
//    case GET_STATUS : {
//      uint8_t u8Data;

//      u8Data = *(uint8_t *)(USBD_SRAM_BASE + USB->BUFSEG) ;
//      if(u8Data == 0x80){                                  //设备属性
//        if (USB_ConfigDescriptor[7] & 0x40) ptr[0] = 1;    // Self-Powered
//        else  ptr[0] = 0;                                  // bus-Powered
//      }
//      else if(u8Data == 0x81)  ptr[0] = 0 ;                //接口属性
//      else if (u8Data == 0x82){                            //端点属性
//        uint32_t u32CfgAddr;
//        uint32_t u32Cfg;
//        uint32_t i;
//
//        u32Cfg = (*(uint8_t *)(USBD_SRAM_BASE + USB->BUFSEG)+4) & 0xF;       //端点地址
//        for(i = 0; i < 5; i++){
//          if(gUsbCtrl.EPInfo[i] == u32Cfg){				//找到对应的端点地址
//            u32Cfg = i;
//            break;
//          }
//        }
//        u32CfgAddr = (uint32_t)(u32Cfg << 4) + (uint32_t)0x40060028; //找到该端点的配置寄存器
//        ptr[0] = *((__IO uint32_t *) (u32CfgAddr)) & USB_CFG_SSTALL? 1: 0 ;
//      }
//      else  return; 
//   			
//      ptr[1] = 0;
//      USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
//      USBD->MXPLD0 = 2;
//      USBD->MXPLD1  = 0 ;                                  //状态阶段准备
//      return ;
//    }

//    case GET_INTERFACE:
//      USBD->EP[0].CFG |= USB_CFG_DSQ_SYNC_DATA1;
//      USBD->EP[0].MXPLD = 1 ;                              
//      USBD->EP[1].MXPLD = 0 ;                              
//      return ;

   case SET_INTERFACE:
          // Does interface need be stored ? 
     USBD->EP[0].CFG |= USB_CFG_DSQ_SYNC_DATA1 ;
     USBD->EP[0].MXPLD = 0;                               //状态阶段准备
     return ;

    default:
      USBD->EP[0].CFGP |= 2 ;                              //功能不支持
      USBD->EP[1].CFGP |= 2 ;
      return ;
  }
}
//// USB 数据收发中断处理, SETUP中断,或8个端点事件中断，调用各自的处理函数 ==============
void USB_UsbEvent(uint32_t u32INTSTS)
{
  if(u32INTSTS & USBD_INTF_SETUP){		   
    USBD->INTSTS = USBD_INTF_SETUP ;     
    switch(pSetUp[0] & 0x60){          //D[6,5]:0=Stardard,1=Class,2=Vendor,3=Reserved 
      case 0:                                              // 0x00, 11种标准请求
        USB_StandardRequest();          
        break;
      case 0x20:                                           // 0x20, 类请求
        USB_ClassRequest();
        break;
      case 0x40:                                           // 0x40, 厂商自定义请求
      default:                                             // 其它不支持, 回应STALL 		   
      USBD->EP[0].CFGP |= 2 ;                              //功能不支持
      USBD->EP[1].CFGP |= 2 ;
    }                      
  }
  else if(u32INTSTS & USBD_INTF_EP0){                      // Ctrl_In,端点地址: 0 
    USB_CtrlIn_Ack();
    USBD->INTSTS = USBD_INTF_EP0;
  }
  else if(u32INTSTS & USBD_INTF_EP1){		                   // Ctrl_Out, 端点地址: 0 	  
    USBD->INTSTS = USBD_INTF_EP1;
    USB_CtrlOut_Ack();
  }
  else if(u32INTSTS & USBD_INTF_EP2){	                     //Bulk_In, 端点地址: 1 
    USBD->INTSTS = USBD_INTF_EP2;
    USB_Tx_OVER() ;
  }
  else if(u32INTSTS & USBD_INTF_EP3){	
    USBD->INTSTS = USBD_INTF_EP3 ;                         //Bulk_Out, 端点地址: 2
    PRT_Flag    &= ~PRINT_RECEIVING ;
#ifdef PRINTER_USB_VCOM                                    
    USB_Rx_Data() ;                                        //处理收到的数据
    USBD->EP[3].MXPLD = USB_PACKET_SIZE_MAX ;              //端点3 Rx准备接收下一包数据
#else
    PTR_Data_Receive() ;                             // 是否接收下一包,在数据处理时判断
#endif    
  }
  else if(u32INTSTS & USBD_INTF_EP4){
    USBD->INTSTS = USBD_INTF_EP4;                          //中断 In , 端点地址: 3

  }	
  else if(u32INTSTS & USBD_INTF_EP5){
    USBD->INTSTS = USBD_INTF_EP5;	
  }	
  else if(u32INTSTS & USBD_INTF_EP6){
    USBD->INTSTS = USBD_INTF_EP6;	
  }	
  else if(u32INTSTS & USBD_INTF_EP7){
    USBD->INTSTS = USBD_INTF_EP7;	
  }	
  else{                                         		
    USBD->INTSTS = USBD_INTF_USB;
  }
}

//== USB 中断代码, 只有USB事件是调用函数, 另三个事件直接处理 ============================
//四个中断源: USB数据收发中断, 总线事件中断, 插拔中断, 唤醒中断. 
void USBD_IRQHandler(void)
{
  uint32_t u32INTSTS = USBD->INTSTS;

  if(u32INTSTS & USBD_INTSTS_USB_STS_Msk){                 //USB 数据收发中断
    USB_UsbEvent(u32INTSTS);                 
  }

  else if(u32INTSTS & USBD_INTSTS_BUS_STS_Msk){            //Bus中断
    if (USBD->ATTR & USBD_ATTR_RESUME_Msk){		             //总线恢复  
      USBD->ATTR  |=  USBD_ATTR_PHY_EN_Msk  ;                        
    }
    else if (USBD->ATTR & USBD_ATTR_SUSPEND_Msk){          //总线挂起 
      USBD->ATTR  &= ~USBD_ATTR_PHY_EN_Msk ;                               
    }
    else if (USBD->ATTR & USBD_ATTR_USBRST_Msk){           //总线复位
      USBD->ATTR |= USBD_ATTR_PHY_EN_Msk ; 		
      USBD->FADDR          = 0 ;	
      gUsbCtrl.ConfigState = 0 ;                  
      gUsbCtrl.StateFlag   = 0 ;
	  }
    USBD->INTSTS |= USBD_INTSTS_BUS_STS_Msk ; 	           //清中断标志
  }

  else if (u32INTSTS & USBD_INTSTS_FLDET_STS_Msk){         //插拔 中断
    USBD->INTSTS |= USBD_INTSTS_FLDET_STS_Msk ;	      
    USBD->ATTR = USBD_ATTR_BYTEM_Msk | USBD_ATTR_PWRDN_Msk | USBD_ATTR_DPPU_EN_Msk | USBD_ATTR_USB_EN_Msk ;   
  }

  else if (u32INTSTS & USBD_INTSTS_WAKEUP_STS_Msk){        //Wake up 中断
    USBD->INTSTS |= USBD_INTSTS_WAKEUP_STS_Msk;            //清标志位
  }
}

// 有Num个字符的字符串*ptr输出到USB虚拟串口 /////////////////////////////////////////////
void Send_to_VCOM(uint8_t *pStr, uint32_t Num)
{
  uint8_t *ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[2].BUFSEG) ;  //USB发送缓存

  Vcom_TxSize = Num ;                                      //发送字符个数
  while(Num--)  *ptr++ = *pStr++ ;                         //把待发送数据移到USB发送缓存
  USBD->EP[2].MXPLD = Vcom_TxSize ;                        //发送开始
}
