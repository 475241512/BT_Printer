#ifndef  __NUC123_usb_1_H
#define  __NUC123_usb_1_H

#define  PRINTER_USB_VCOM

#ifdef  USB_GLOBAL 
  #define EXT_USB
#else
  #define EXT_USB  extern
#endif

typedef struct DRVUSBD_CTRL_STRUCT{	 
	uint32_t  ReqLen;                //请求数据长度, 长于最大包时, 多次收发计数
	uint8_t  *pDesc;                 //数据多于一整包时, 下次发送的数据首地址
	uint32_t  StateFlag;             //在状态阶段,以此判断数据阶段做了哪些事 
	uint8_t   ConfigState;           //有多个配置时,这个保存当前配置,为0时表示未配置
}STR_USBD_CTRL_T ;
EXT_USB volatile  STR_USBD_CTRL_T   gUsbCtrl ;

EXT_USB  volatile  uint32_t  Vcom_TxSize ;       //为0表示可以发送. 非零表示VCOM在忙

void USB_Init(void) ;
void EP3_Rx_Ready(void);
void Send_to_VCOM(uint8_t *pStr, uint32_t Num) ;

#endif   //__Nano1xx_usb_1_H
