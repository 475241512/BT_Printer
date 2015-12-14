#include "includes.h"

#define F25L_NCS_OUTPUT()	do{	\
	}while(0)

#define F25L_NCS_HIGH()	 do{ \
       LPC_GPIO1->MASKED_ACCESS[0x100] = 0x100; \
       }while(0)

#define F25L_NCS_LOW()	 do{  \
       LPC_GPIO1->MASKED_ACCESS[0x100] = 0x000;  \
       }while(0)

#define F25L_SCLK_OUTPUT()	do{	\
	}while(0)

#define F25L_SCLK_HIGH()   do{  \
       LPC_GPIO1->MASKED_ACCESS[0x200] = 0x200;  \
       }while(0)

#define F25L_SCLK_LOW()	   do{  \
       LPC_GPIO1->MASKED_ACCESS[0x200] = 0x000;  \
       }while(0)

#define F25L_MOSI_OUTPUT()	do{	\
	  }while(0)

#define F25L_MOSI_HIGH()   do{ \
      LPC_GPIO1->MASKED_ACCESS[0x10] = 0x10; \
     }while(0)

#define F25L_MOSI_LOW()    do{ \
      LPC_GPIO1->MASKED_ACCESS[0x10] = 0x00;  \
      }while(0)

#define F25L_MOSI_OUT(data)	do{ \
		GPIOSetValue(PORT1,GPIO_Pin_4,data);			\
	  }while(0)

#define F25L_MISO_HIGH() do{ \
      LPC_GPIO1->MASKED_ACCESS[0x08]=0x08; \
     }while(0)

#define F25L_MISO_INPUT()	do{ \
	}while(0)

#define F25L_MISO_READ()  (LPC_GPIO1->MASKED_ACCESS[0x08])




#define opcodeStatusRegisterRead								(0x05)
	#define BUSY												(1 << 0)
	#define WEL													(1 << 1)
	#define BP0													(1 << 2)
	#define BP1 												(1 << 3)
	#define BP2 												(1 << 4)
	#define RESERVED											(1 << 5)
	#define AAI													(1 << 6)
	#define BPL													(1 << 7)

#define opcodePageProgram                                       (0x02)
#define opcodeWriteEnable										(0x06)
#define opcodeWriteDisable										(0x04)
#define opcodeBusyOutputEnable									(0x70)
#define opcodeBusyOutputDisable									(0x80)
#define opcodeStatusRegisterWriteEnable							(0x50)
#define opcodeStatusRegisterWrite								(0x01)
#define opcodeReadElectronicSignature							(0xab)
#define opcodeReadJedecId										(0x9f)
#define opcodeReadId											(0x90)

#define opcodeRead												(0x03)
#define opcodeFastRead											(0x0b)
#define opcodeByteProgram										(0x02)
#define opcodeAAIWordProgram									(0xad)
#define opcodeBlockErase										(0xd8)
#define opcodeSectorErase										(0x20)
#define opcodeChipErase											(0xc7)



static void F25L_ShiftOutByte(uint8_t dat)
{
	uint8_t i;


	i = 7;
	do
	{
		F25L_SCLK_LOW();
//     LPC_GPIO1->MASKED_ACCESS[0x200] = 0x000;
	 if(dat & (1<<i))
	 {
       F25L_MOSI_HIGH();
//      LPC_GPIO1->MASKED_ACCESS[0x10] = 0x10;
	 }
       else
       {
        F25L_MOSI_LOW();
 //      LPC_GPIO1->MASKED_ACCESS[0x10] = 0;
       }
		F25L_SCLK_HIGH();
//       LPC_GPIO1->MASKED_ACCESS[0x200] = 0x200;
	}while(i--);
}


    static uint8_t F25L_ShiftInByte(void)
    {
        uint8_t i;
        uint8_t d;

        d = 0;
        i = 7;
        do
        {
          F25L_SCLK_LOW();

//            LPC_GPIO1->MASKED_ACCESS[0x200] = 0x00;
          F25L_SCLK_HIGH();

//            LPC_GPIO1->MASKED_ACCESS[0x200] = 0x200;

//         if(LPC_GPIO1->MASKED_ACCESS[0x08])
          if (F25L_MISO_READ())
            {
                d |= (1<<i);
            }
        }while (i--);

        return d;
    }


static void F25L_ShiftOutAddrWrite(uint8_t opcode, uint32_t addr)
{
	F25L_ShiftOutByte(opcode);
	F25L_ShiftOutByte(addr >> 16);
	F25L_ShiftOutByte(addr >> 8);
	F25L_ShiftOutByte(addr >> 0);
}

extern uint8_t F25L_StatusRegisterRead(void)
{
	uint8_t ret;

	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeStatusRegisterRead);
	ret = F25L_ShiftInByte();
	F25L_NCS_HIGH();
	return ret;
}

static void F25L_WriteEnable(void)
{
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeWriteEnable);
	F25L_NCS_HIGH();
}

static void F25L_WriteDisable(void)
{
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeWriteDisable);
	F25L_NCS_HIGH();
}
#if 0
static void F25L_BusyOutputEnable(void)
{
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeBusyOutputEnable);
	F25L_NCS_HIGH();
}
#endif
#if 0
static void F25L_BusyOutputDisable(void)
{
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeBusyOutputDisable);
	F25L_NCS_HIGH();
}
#endif
static void F25L_StatusRegisterWriteEnable(void)
{
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeStatusRegisterWriteEnable);
	F25L_NCS_HIGH();
}

static void F25L_StatusRegisterWrite(uint8_t status)
{
	F25L_StatusRegisterWriteEnable();

	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeStatusRegisterWrite);
	F25L_ShiftOutByte(status);
	F25L_NCS_HIGH();
}

extern void F25L_Init(void)
{
#if 0
    LPC_IOCON->PIO1_8=0xD0;
    GPIOSetDir(PORT1,GPIO_Pin_8,Output);


    LPC_IOCON->PIO1_9=0xD0;
    GPIOSetDir(PORT1,GPIO_Pin_9,Output);

    LPC_IOCON->SWDIO_PIO1_3=0xD1;
    GPIOSetDir(PORT1,GPIO_Pin_3,Input);

    LPC_IOCON->PIO1_4 =0xD0;
    GPIOSetDir(PORT1,GPIO_Pin_4,Output);

	F25L_NCS_HIGH();
	F25L_NCS_OUTPUT();

	F25L_SCLK_HIGH();
	F25L_SCLK_OUTPUT();

	F25L_MOSI_HIGH();
	F25L_MOSI_OUTPUT();

    F25L_MISO_HIGH();
	F25L_MISO_INPUT();
#endif
}
#if 1
extern void F25L_ReadJedecId(uint8_t  *buf)
{
#if 0
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeReadJedecId);
	*buf++ = F25L_ShiftInByte();
	*buf++ = F25L_ShiftInByte();
	*buf = F25L_ShiftInByte();
	F25L_NCS_HIGH();
#endif
}
#endif

extern void F25L_ReadId(uint8_t  *buf)
{
#if 0
	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeReadId);
	F25L_ShiftOutByte(0);
	F25L_ShiftOutByte(0);
	F25L_ShiftOutByte(0);
	*buf++ = F25L_ShiftInByte();
	*buf = F25L_ShiftInByte();
	F25L_NCS_HIGH();
#endif
}

#if 0
extern uint8_t F25L_ReadElectronicSignature(void)
{
	uint8_t ret;

	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeReadElectronicSignature);
	ret = F25L_ShiftInByte();
	F25L_NCS_HIGH();

	return ret;
}
#endif

extern void F25L_Read(uint32_t addr, uint8_t  *buf, uint16_t bytes)
{
#if 0
	F25L_NCS_LOW();
//    LPC_GPIO1->MASKED_ACCESS[0x100] = 0x000;

	F25L_ShiftOutAddrWrite(opcodeRead, addr);

	for(; bytes; bytes--)
	{
		*buf++ = F25L_ShiftInByte();
	}
	F25L_NCS_HIGH();
  //  LPC_GPIO1->MASKED_ACCESS[0x100] = 0x100;
#endif
}


extern void F25L_FastRead(uint32_t addr, uint8_t  *buf, uint16_t bytes)
{
#if 0
	F25L_NCS_LOW();
	F25L_ShiftOutAddrWrite(opcodeFastRead, addr);
	F25L_ShiftOutByte(0xff);	// dummy
	for(; bytes; bytes--)
	{
		*buf++ = F25L_ShiftInByte();
	}
	F25L_NCS_HIGH();
#endif
}


//#if 1
extern void F25L_ByteProgram(uint32_t addr, uint8_t dat)
{
#if 0
	F25L_StatusRegisterWrite(0x00);


	F25L_WriteEnable();
	F25L_NCS_LOW();
	F25L_ShiftOutAddrWrite(opcodeByteProgram, addr);
	F25L_ShiftOutByte(dat);
	F25L_NCS_HIGH();
	while (F25L_StatusRegisterRead() & BUSY);
#endif
}
//#endif

extern void F25L_AAIWordProgram(uint32_t addr, uint8_t  *dat, uint16_t words)
{
#if 0
	if(words)
	{
		F25L_StatusRegisterWrite(0x00);

		F25L_WriteEnable();
		F25L_NCS_LOW();
		F25L_ShiftOutAddrWrite(opcodeAAIWordProgram, addr);
		F25L_ShiftOutByte(*dat++);
		F25L_ShiftOutByte(*dat++);
		F25L_NCS_HIGH();
		while (F25L_StatusRegisterRead() & BUSY);
		words--;
		for(; words; words--)
		{
			F25L_NCS_LOW();
			F25L_ShiftOutByte(opcodeAAIWordProgram);
			F25L_ShiftOutByte(*dat++);
			F25L_ShiftOutByte(*dat++);
			F25L_NCS_HIGH();
			while (F25L_StatusRegisterRead() & BUSY);
		}
		F25L_WriteDisable();
		while (F25L_StatusRegisterRead() & BUSY);
	}
#endif
}
extern void F16_PageProgram(uint32_t addr, uint8_t *dat, uint32_t len)
{
#if 0
    if(len && (len <=256))
    {
        F25L_StatusRegisterWrite(0x00);
        F25L_WriteEnable();
		F25L_NCS_LOW();
		F25L_ShiftOutAddrWrite(opcodePageProgram, addr);
        while(len--)
        {
            F25L_ShiftOutByte(*dat++);
        }
        F25L_NCS_HIGH();
		while (F25L_StatusRegisterRead() & BUSY);
    }
#endif
}

#if 0
extern void F25L_BlockErase(uint16_t block)
{
	F25L_StatusRegisterWrite(0x00);

	F25L_WriteEnable();

	F25L_NCS_LOW();
	F25L_ShiftOutAddrWrite(opcodeBlockErase, (block << 16));
	F25L_NCS_HIGH();

	while (F25L_StatusRegisterRead() & BUSY);
}
#endif

//#if 1
extern void F25L_SectorErase(uint16_t sector)
{
#if 0
	F25L_StatusRegisterWrite(0x00);
	F25L_WriteEnable();
	F25L_NCS_LOW();
    F25L_ShiftOutAddrWrite(opcodeSectorErase, (sector << 12));
	F25L_NCS_HIGH();
	while (F25L_StatusRegisterRead() & BUSY);
#endif
}
//#endif

extern void F25L_ChipErase(void)
{
#if 0
	F25L_StatusRegisterWrite(0x00);

	F25L_WriteEnable();

	F25L_NCS_LOW();
	F25L_ShiftOutByte(opcodeChipErase);
	F25L_NCS_HIGH();

	while (F25L_StatusRegisterRead() & BUSY);
#endif
}


