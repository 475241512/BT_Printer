#ifndef _SSP_H
#define _SSP_H

#define SR_TFE (1<<0)
#define SR_TNF (1<<1)
#define SR_RNE (1<<2)
#define SR_RFF (1<<3)
#define SR_BSY (1<<4)

#define SSP0   0
#define SSP1   1


extern void SSP_INIT(uint8_t ssp_num);
extern void SSP0_SendData(uint8_t data);







#endif
