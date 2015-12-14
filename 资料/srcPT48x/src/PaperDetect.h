#ifndef __PAPERDETECT_H__
#define __PAPERDETECT_H__

#define AD0                0
#define AD1                1
#define AD2                2
#define AD7                7

#define ADCCLKDIV          (11<<8)
#define ADCCLKS            (0x0<<17)
#define STARTMode          (0x0<<24)
#define SofewareMode       (0x0<<16)
#define HardwareMode       (0x1<<16)

extern volatile uint16_t Power_AD;

extern uint8_t TPPrinterMark(void);
extern void PaperPlatenSNSInit(void);
extern uint8_t TPPrinterReady(void);
//extern uint8_t TPPaperReady(void);
extern void TPPaperSNSDetect(uint8_t c);
extern void TPPaperSNSInit(void);

extern void PaperStartSns(void);

extern uint8_t TPPaperReady(void);

#endif


