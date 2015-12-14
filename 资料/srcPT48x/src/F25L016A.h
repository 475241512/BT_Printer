#ifndef __F25L016A_H__
#define __F25L016A_H__
//======================================================================================================
extern void F25L_Init(void);
extern void F25L_ReadJedecId(uint8_t  *buf);
extern void F25L_ReadId(uint8_t  *buf);
extern uint8_t F25L_ReadElectronicSignature(void);
extern void F25L_Read(uint32_t addr, uint8_t  *buf, uint16_t bytes);
extern void F25L_FastRead(uint32_t addr, uint8_t  *buf, uint16_t bytes);
extern void F25L_ByteProgram(uint32_t addr, uint8_t dat);
extern void F25L_AAIWordProgram(uint32_t addr, uint8_t  *dat, uint16_t words);
extern void F25L_BlockErase(uint16_t block);
extern void F25L_SectorErase(uint16_t sector);
extern void F25L_ChipErase(void);
extern void F16_PageProgram(uint32_t addr, uint8_t *dat, uint32_t len);
extern uint8_t F25L_StatusRegisterRead(void);


//======================================================================================================
#endif

