//=======================================================================================
//  Copyright Nuvoton(Shang-Hai) Technology Corp. All rights reserved. 2014 
//=======================================================================================
#define   USB_GLOBAL
#include  "NUC123Series.h"
#include  "printer.h"
#include  "Nuc123_usb_1.h" 

extern  void  USB_Tx_OVER(void) ;                //�൱�ڷ��ͽ����жϺ���
extern  void  USB_Rx_Data(void) ;                //�൱�ڽ��յ������жϺ���
extern  void  PTR_Data_Receive(void) ;              

#define  GET_STATUS                0x00                    //USBЭ�鶨���11�ֱ�׼����
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

#define  DESC_DEVICE               0x01                    //USBЭ�鶨��ļ�������������
#define  DESC_CONFIG               0x02
#define  DESC_STRING               0x03
#define  DESC_INTERFACE            0x04
#define  DESC_ENDPOINT             0x05
#define  DESC_QUALIFIER            0x06
#define  DESC_OTHERSPEED           0x07

#define  LEN_DEVICE                18                      //USBЭ�鶨����豸����������
#define  LEN_CONFIG                9                       //USBЭ�鶨�����������������
#define	 LEN_INTERFACE             9                       //USBЭ�鶨��Ľӿ�����������
#define	 LEN_ENDPOINT              7                       //USBЭ�鶨��Ķ˵�����������

#define  EP_BULK                   0x02                    //���ĸ���USBЭ���ж���Ķ˵�����ֵ
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
                                                           //����д�˵����� CFG ��λ����
#define  USB_OUT_ENDP              0x20                    //����˵�
#define  USB_ISOCH_OUT_ENDP        0x30                    //��ʱ����˵�
#define  USB_IN_ENDP               0x40                    //����˵�     
#define  USB_ISOCH_IN_ENDP         0x50                    //��ʱ����˵�

#define  USB_PACKET_SIZE_MAX       64                      // ���USB����С  
#define  USB_CFG_DSQ_SYNC_DATA1    0x0080                  // �������� DATA1

#define  BULK_IN_EP_NUM            1                       //�õ��������˵��ַ
#define  BULK_OUT_EP_NUM           2
#define  INT_IN_EP_NUM             3 

#ifdef PRINTER_USB_VCOM    ////=========================================// ���⴮��������

#define  USB_VID		           0x0416
#define  USB_PID		           0x5011
const uint8_t USB_DeviceDescriptor[] =
{
	LEN_DEVICE,		/* bLength              */
	DESC_DEVICE,	/* bDescriptorType      */
	0x00, 0x02,		/* bcdUSB               */                 // USB�汾
	0x02,			/* bDeviceClass         */                     // �豸��
	0x00,			/* bDeviceSubClass      */
	0x00,			/* bDeviceProtocol      */
	USB_PACKET_SIZE_MAX,	/* bMaxPacketSize0 */              // �˵�0����
	/* idVendor */
	USB_VID & 0x00FF,                                        // ���̱��
	(USB_VID & 0xFF00) >> 8,
	/* idProduct */
	USB_PID & 0x00FF,                                        // ��Ʒ���
	(USB_PID & 0xFF00) >> 8,
	0x00, 0x03,		/* bcdDevice            */                 // �豸���
	0x01,			/* iManufacture         */                     // ���̴�����
	0x02,			/* iProduct             */                     // ��Ʒ������
	0x00,			/* iSerialNumber        */                     // ���кŴ�����
	0x01			/* bNumConfigurations   */
};
const uint8_t USB_ConfigDescriptor[] =
{
	LEN_CONFIG,		/* bLength              */
	DESC_CONFIG,	/* bDescriptorType      */
	0x43, 0x00,		/* wTotalLength         */                 //�����������ܳ���
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
	(EP_INPUT | INT_IN_EP_NUM),     /* bEndpointAddress */   //�˵��ַ:3,�ж�IN
	EP_INT,		                    /* bmAttributes     */
	USB_PACKET_SIZE_MAX, 0x00,	    /* wMaxPacketSize   */
	0x01,	                        /* bInterval        */     //��� 1ms
			
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
	(EP_INPUT | BULK_IN_EP_NUM),	/* bEndpointAddress */     //�˵��ַ:1, Bulk in
	EP_BULK,		                /* bmAttributes     */
	USB_PACKET_SIZE_MAX, 0x00,	    /* wMaxPacketSize   */
	0x00,			                /* bInterval        */

	/* ENDPOINT descriptor */
	LEN_ENDPOINT,	                /* bLength          */
	DESC_ENDPOINT,	                /* bDescriptorType  */
	(EP_OUTPUT | BULK_OUT_EP_NUM),	/* bEndpointAddress */   //�˵��ַ:2, Bulk out
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

#else     ////=============================================// ��ӡ�������� ==============
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

//�����֧���ַ�������,��Щ�ַ����Ϳ��Բ�����,��ʡ����ռ�
const uint8_t USB_StringLang[4] =                          // ������
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

//������ָ��궨�壬ע��Ҫ�ͳ�ʼ��SRAM����ĵ�ַһ��    
#define  pSetUp              ((uint8_t*)USB_SRAM)          //SETUP���ݷ���SRAM 0
#define  pEnd0Data           ((uint8_t*)(USB_SRAM+8))      //�˵�0/1���ݷ���SRAM 8

//// USB ��ʼ�� =========================================================================
void USB_Init(void)
{
  uint32_t volatile temp  = 900 ;
 
  CLK->APBCLK  |=  CLK_APBCLK_USBD_EN_Msk ;                // Enable USB Clock
  SYS->IPRSTC2 |=  (1ul << 27) ;                           // Reset USB 
  SYS->IPRSTC2 &= ~(1ul << 27) ;

  USBD->ATTR |= USBD_ATTR_BYTEMODE ;
  USBD->DRVSE0 = 1 ;	
  while(temp--) ;   
  
  USBD->STBUFSEG  = 0 ;                                           //Setup��8���ֽڴ����ַ                           
  USBD->EP[0].BUFSEG = 8 ;				                                //������ַ
  USBD->EP[0].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP) ;    //�˵��ַ:0
  USBD->EP[1].BUFSEG = 8 ;				                  
  USBD->EP[1].CFG    = (USBD_EPCFG_CLEARSTALL | USB_OUT_ENDP) ;   //�˵��ַ��Ϊ0,�ҹ���һ��������

  USBD->EP[2].BUFSEG = 8+64 ;				                                            //������ַ
  USBD->EP[2].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP | BULK_IN_EP_NUM);  //�˵��ַ:1,BULK_IN_EP_NUM
  USBD->EP[3].BUFSEG = 8+64+64 ;                                                //������ַ
  USBD->EP[3].CFG    = (USBD_EPCFG_CLEARSTALL | USB_OUT_ENDP | BULK_OUT_EP_NUM);//�˵��ַ:2,BULK_OUT_EP_NUM
  USBD->EP[4].BUFSEG = 8+64+64+64 ;                                             //������ַ
  USBD->EP[4].CFG    = (USBD_EPCFG_CLEARSTALL | USB_IN_ENDP | INT_IN_EP_NUM) ;  //�˵��ַ:3,INT_IN_EP_NUM

  USBD->INTEN = USBD_WAKUP_IE_ENABLE | USBD_INT_FLDET | USBD_INT_USB | USBD_INT_BUS ;
  NVIC_SetPriority(USBD_IRQn, 2) ;                                   // �ж����ȼ�Ϊ 2, ������ֽ����ж�
  NVIC->ISER[0] = 1 << USBD_IRQn ;                                   // ���ж�����
  USBD->DRVSE0 = 0 ;
  Vcom_TxSize  = 0 ;                                                 //��ʾû������Ҫ����

  PDMA2->SAR  = USBD->EP[3].BUFSEG ;                       // Դ��ַ: EP[3]�� RAM
  PDMA2->CSR  = 0x00000025 +(1 << 19) ;                    // 8λ,Դ�̶�,Ŀ�ļ�,IP->MEM  
  PDMA2->CSR |=  2 ;                                       // ��λ PDMA0
}

void USB_CtrlOut_Ack(void)
{
  switch(gUsbCtrl.StateFlag){                                    
    case FLAG_SET_LINE_CODE :
//      if(gUsbInfo.UsbDeviceClass & USB_CLASS_CDC){
//        MultiByte_copy((void *)&gCdcInfo,(uint8_t*)(USBD_SRAM_BASE+USBD->BUFSEG0),USBD->MXPLD1);
//        USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1 ;
//        USBD->MXPLD0 = 0 ;                                 //״̬�׶�׼��
//      }
      break ;

    case 0 :                       //״̬�׶�OUT_ACK�����ϴ��޴�,�����������ᷢ���ش�
    default :
      break ;
  }
  gUsbCtrl.StateFlag = 0;
}

// �˵�0��Ctrl_IN����,�����ƴ����״̬�׶� ==============================================
void USB_CtrlIn_Ack(void)
{
  uint32_t  Cnt ;

  switch (gUsbCtrl.StateFlag){
    case FLAG_EP0_DATA_IN :                                //�ϴ�������������
      if(gUsbCtrl.ReqLen){
        Cnt = 0 ;
        do{
          pEnd0Data[Cnt] = *gUsbCtrl.pDesc++ ;
          if(++Cnt >= USB_PACKET_SIZE_MAX) break ;
        }while(--gUsbCtrl.ReqLen) ;
        USBD->EP[0].MXPLD = Cnt ;
      }
      else{                                                // ������ȫ���ϴ�
        USBD->EP[0].MXPLD  = 0 ;
        USBD->EP[1].MXPLD  = 0 ;                           // ����״̬�׶�
        gUsbCtrl.StateFlag = 0 ;                           // ״̬������,�������� 
      }	
      return ;
                                                           //������״̬�׶ε� IN_ACK 
    case FLAG_SET_ADDRESS :                                //���õ�ַ���̽���
        USBD->FADDR = pSetUp[2];                           //����ʡȥ��StatusFlag����
    default:
        return;    
  }
}

//// ��������, Ϊ�򻯴���,������ܿ��Բ�֧�� ==========================================
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
    
      USBD->EP[1].MXPLD = 0 ;                              // ״̬�׶ε�OUT׼��
      break ;
    
    case SET_LINE_CODE :                                  
      USBD->EP[1].MXPLD = 7 ;                              // Out�����ʵ����ݶ��� 
    
      USBD->EP[0].CFG  |= 0x80;   
      USBD->EP[0].MXPLD = 0;                               // ״̬�׶�׼��
      break ;
    
    case SET_CONTROL_LINE_STATE : 
      gVCOMStatus = ((uint16_t)pSetUp[3]) + pSetUp[2] ;  
    
      USBD->EP[0].CFG  |= 0x80;   
      USBD->EP[0].MXPLD = 0;                               // ״̬�׶�׼��
      break ;
    
//     case GET_PORT_STATUS :                                 // In PtrStatus
//       USBD->EP[0].CFG  |= USB_CFG_DSQ_SYNC_DATA1 ;
//       USBD->EP[0].MXPLD = 0 ;                              //״̬�׶�׼��
//       break ;
    default :
    USBD->EP[0].CFGP |= 2 ;                                //���ܲ�֧��,��STALL
    USBD->EP[1].CFGP |= 2 ;
  }
}

/// ��������������, Ϊ�򻯴���, ȡ�����ַ��������� ====================================
void USB_GetDescriptor(void)
{
  uint32_t Cnt ;
  uint8_t *ptr ;

  gUsbCtrl.ReqLen = pSetUp[6] + ((uint32_t)pSetUp[7]<<8);  //���䳤��
  USBD->EP[0].CFG |= 0x80;                                 //֮ǰ��SETUP,��λ����Toggle
  gUsbCtrl.StateFlag = 0 ;             //�ȼٶ�������������״̬�׶�,��״̬����ʲôҲ����
            
  switch(pSetUp[3]){                                       //����ֽھ�����ʲô������
    case DESC_DEVICE:                                      //�豸������, 18�ֽ�
      ptr = (uint8_t*)USB_DeviceDescriptor ;
      Cnt = 0 ;
      do{
        pEnd0Data[Cnt] = *ptr++ ;
      }while(++Cnt <= 18);                                 //�豸������18���ֽ�
      USBD->EP[0].MXPLD = 18 ;
      break;
	 
    case DESC_CONFIG:                            //����������, ���Ȳ���, ��67���ֽ�
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
        USBD->EP[1].MXPLD  = 0 ;                           // ״̬�׶�׼��
      }
      break;

   case DESC_STRING:                                       //��������
     Cnt = 0 ;
     if(pSetUp[2] == 0){                                   // ������
       gUsbCtrl.ReqLen = Minimum (gUsbCtrl.ReqLen, USB_StringLang[0]);
       ptr = (uint8_t*)USB_StringLang ;
       do{
         pEnd0Data[Cnt] = *ptr++ ;
       }while(++Cnt <= gUsbCtrl.ReqLen);
       USBD->EP[0].MXPLD = gUsbCtrl.ReqLen;
       break;
     }
//      else if(pSetUp[2] == 1){                              // ��Ӧ��
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
     else if(pSetUp[2] == 2){                              // �豸��
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
      USBD->EP[0].CFGP |= 2 ;                              //���ܲ�֧��
      USBD->EP[1].CFGP |= 2 ;
      return ;
	}
    USBD->EP[1].MXPLD = 0;              //״̬�׶�׼��, ��Щ����IN����, ����״̬�׶ε�OUT׼��
	return ;
}

void EP3_Rx_Ready(void)
{
  USBD->EP[3].MXPLD = USB_PACKET_SIZE_MAX ;                //�˵�3 Rx׼��������һ������
  PRT_Flag |= PRINT_RECEIVING ;
}
//// 11�ֱ�׼����,����ֻ�����õ��ļ���,�ɼ򻯴��� =======================================
void USB_StandardRequest(void)
{
  gUsbCtrl.StateFlag = 0 ;             //�ȼٶ�������������״̬�׶�,��״̬����ʲôҲ����

  switch (pSetUp[1]){                                      //����������
    case GET_DESCRIPTOR:
      USB_GetDescriptor() ; 
      return  ;

    case SET_ADDRESS:                                      //�趨��ַ
      gUsbCtrl.StateFlag = FLAG_SET_ADDRESS;               //�����ݽ׶�
      USBD->EP[0].CFG |= USB_CFG_DSQ_SYNC_DATA1 ;    
      USBD->EP[0].MXPLD = 0;                               //״̬�׶�׼��
      return; 
 
    case SET_CONFIGURATION:                                //�趨����ֵ
      gUsbCtrl.ConfigState = pSetUp[2] ;                   //��������ֵ
      USBD->EP[2].MXPLD = 0;                               //�˵�2 Tx����0��
      EP3_Rx_Ready() ;
      USBD->EP[0].CFG  |= USB_CFG_DSQ_SYNC_DATA1 ;   
      USBD->EP[0].MXPLD = 0 ;                              //״̬�׶�׼��
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
//      *ptr = gUsbCtrl.ConfigState ;                        //�ͷ���һ���ֽ�:����ֵ
//      USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1 ;
//      USBD->MXPLD0  = 1 ;                                  //��������׼��
//      USBD->MXPLD1  = 0 ;                                  //״̬�׶�׼��
//      return ; 
 
//    case GET_STATUS : {
//      uint8_t u8Data;

//      u8Data = *(uint8_t *)(USBD_SRAM_BASE + USB->BUFSEG) ;
//      if(u8Data == 0x80){                                  //�豸����
//        if (USB_ConfigDescriptor[7] & 0x40) ptr[0] = 1;    // Self-Powered
//        else  ptr[0] = 0;                                  // bus-Powered
//      }
//      else if(u8Data == 0x81)  ptr[0] = 0 ;                //�ӿ�����
//      else if (u8Data == 0x82){                            //�˵�����
//        uint32_t u32CfgAddr;
//        uint32_t u32Cfg;
//        uint32_t i;
//
//        u32Cfg = (*(uint8_t *)(USBD_SRAM_BASE + USB->BUFSEG)+4) & 0xF;       //�˵��ַ
//        for(i = 0; i < 5; i++){
//          if(gUsbCtrl.EPInfo[i] == u32Cfg){				//�ҵ���Ӧ�Ķ˵��ַ
//            u32Cfg = i;
//            break;
//          }
//        }
//        u32CfgAddr = (uint32_t)(u32Cfg << 4) + (uint32_t)0x40060028; //�ҵ��ö˵�����üĴ���
//        ptr[0] = *((__IO uint32_t *) (u32CfgAddr)) & USB_CFG_SSTALL? 1: 0 ;
//      }
//      else  return; 
//   			
//      ptr[1] = 0;
//      USBD->CFG0 |= USB_CFG_DSQ_SYNC_DATA1;
//      USBD->MXPLD0 = 2;
//      USBD->MXPLD1  = 0 ;                                  //״̬�׶�׼��
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
     USBD->EP[0].MXPLD = 0;                               //״̬�׶�׼��
     return ;

    default:
      USBD->EP[0].CFGP |= 2 ;                              //���ܲ�֧��
      USBD->EP[1].CFGP |= 2 ;
      return ;
  }
}
//// USB �����շ��жϴ���, SETUP�ж�,��8���˵��¼��жϣ����ø��ԵĴ����� ==============
void USB_UsbEvent(uint32_t u32INTSTS)
{
  if(u32INTSTS & USBD_INTF_SETUP){		   
    USBD->INTSTS = USBD_INTF_SETUP ;     
    switch(pSetUp[0] & 0x60){          //D[6,5]:0=Stardard,1=Class,2=Vendor,3=Reserved 
      case 0:                                              // 0x00, 11�ֱ�׼����
        USB_StandardRequest();          
        break;
      case 0x20:                                           // 0x20, ������
        USB_ClassRequest();
        break;
      case 0x40:                                           // 0x40, �����Զ�������
      default:                                             // ������֧��, ��ӦSTALL 		   
      USBD->EP[0].CFGP |= 2 ;                              //���ܲ�֧��
      USBD->EP[1].CFGP |= 2 ;
    }                      
  }
  else if(u32INTSTS & USBD_INTF_EP0){                      // Ctrl_In,�˵��ַ: 0 
    USB_CtrlIn_Ack();
    USBD->INTSTS = USBD_INTF_EP0;
  }
  else if(u32INTSTS & USBD_INTF_EP1){		                   // Ctrl_Out, �˵��ַ: 0 	  
    USBD->INTSTS = USBD_INTF_EP1;
    USB_CtrlOut_Ack();
  }
  else if(u32INTSTS & USBD_INTF_EP2){	                     //Bulk_In, �˵��ַ: 1 
    USBD->INTSTS = USBD_INTF_EP2;
    USB_Tx_OVER() ;
  }
  else if(u32INTSTS & USBD_INTF_EP3){	
    USBD->INTSTS = USBD_INTF_EP3 ;                         //Bulk_Out, �˵��ַ: 2
    PRT_Flag    &= ~PRINT_RECEIVING ;
#ifdef PRINTER_USB_VCOM                                    
    USB_Rx_Data() ;                                        //�����յ�������
    USBD->EP[3].MXPLD = USB_PACKET_SIZE_MAX ;              //�˵�3 Rx׼��������һ������
#else
    PTR_Data_Receive() ;                             // �Ƿ������һ��,�����ݴ���ʱ�ж�
#endif    
  }
  else if(u32INTSTS & USBD_INTF_EP4){
    USBD->INTSTS = USBD_INTF_EP4;                          //�ж� In , �˵��ַ: 3

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

//== USB �жϴ���, ֻ��USB�¼��ǵ��ú���, �������¼�ֱ�Ӵ��� ============================
//�ĸ��ж�Դ: USB�����շ��ж�, �����¼��ж�, ����ж�, �����ж�. 
void USBD_IRQHandler(void)
{
  uint32_t u32INTSTS = USBD->INTSTS;

  if(u32INTSTS & USBD_INTSTS_USB_STS_Msk){                 //USB �����շ��ж�
    USB_UsbEvent(u32INTSTS);                 
  }

  else if(u32INTSTS & USBD_INTSTS_BUS_STS_Msk){            //Bus�ж�
    if (USBD->ATTR & USBD_ATTR_RESUME_Msk){		             //���߻ָ�  
      USBD->ATTR  |=  USBD_ATTR_PHY_EN_Msk  ;                        
    }
    else if (USBD->ATTR & USBD_ATTR_SUSPEND_Msk){          //���߹��� 
      USBD->ATTR  &= ~USBD_ATTR_PHY_EN_Msk ;                               
    }
    else if (USBD->ATTR & USBD_ATTR_USBRST_Msk){           //���߸�λ
      USBD->ATTR |= USBD_ATTR_PHY_EN_Msk ; 		
      USBD->FADDR          = 0 ;	
      gUsbCtrl.ConfigState = 0 ;                  
      gUsbCtrl.StateFlag   = 0 ;
	  }
    USBD->INTSTS |= USBD_INTSTS_BUS_STS_Msk ; 	           //���жϱ�־
  }

  else if (u32INTSTS & USBD_INTSTS_FLDET_STS_Msk){         //��� �ж�
    USBD->INTSTS |= USBD_INTSTS_FLDET_STS_Msk ;	      
    USBD->ATTR = USBD_ATTR_BYTEM_Msk | USBD_ATTR_PWRDN_Msk | USBD_ATTR_DPPU_EN_Msk | USBD_ATTR_USB_EN_Msk ;   
  }

  else if (u32INTSTS & USBD_INTSTS_WAKEUP_STS_Msk){        //Wake up �ж�
    USBD->INTSTS |= USBD_INTSTS_WAKEUP_STS_Msk;            //���־λ
  }
}

// ��Num���ַ����ַ���*ptr�����USB���⴮�� /////////////////////////////////////////////
void Send_to_VCOM(uint8_t *pStr, uint32_t Num)
{
  uint8_t *ptr = (uint8_t *)(USBD_BASE + 0x0100 + USBD->EP[2].BUFSEG) ;  //USB���ͻ���

  Vcom_TxSize = Num ;                                      //�����ַ�����
  while(Num--)  *ptr++ = *pStr++ ;                         //�Ѵ����������Ƶ�USB���ͻ���
  USBD->EP[2].MXPLD = Vcom_TxSize ;                        //���Ϳ�ʼ
}
