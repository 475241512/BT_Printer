#ifndef  __NUC123_usb_1_H
#define  __NUC123_usb_1_H

#define  PRINTER_USB_VCOM

#ifdef  USB_GLOBAL 
  #define EXT_USB
#else
  #define EXT_USB  extern
#endif

typedef struct DRVUSBD_CTRL_STRUCT{	 
	uint32_t  ReqLen;                //�������ݳ���, ��������ʱ, ����շ�����
	uint8_t  *pDesc;                 //���ݶ���һ����ʱ, �´η��͵������׵�ַ
	uint32_t  StateFlag;             //��״̬�׶�,�Դ��ж����ݽ׶�������Щ�� 
	uint8_t   ConfigState;           //�ж������ʱ,������浱ǰ����,Ϊ0ʱ��ʾδ����
}STR_USBD_CTRL_T ;
EXT_USB volatile  STR_USBD_CTRL_T   gUsbCtrl ;

EXT_USB  volatile  uint32_t  Vcom_TxSize ;       //Ϊ0��ʾ���Է���. �����ʾVCOM��æ

void USB_Init(void) ;
void EP3_Rx_Ready(void);
void Send_to_VCOM(uint8_t *pStr, uint32_t Num) ;

#endif   //__Nano1xx_usb_1_H
