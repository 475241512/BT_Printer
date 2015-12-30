#ifdef LCD_VER
#include "Lcd.h"
#include "TimeBase.h"
#include "AsciiLib_6x12.h"

extern unsigned char const fontb_en[];

static unsigned char Lcd_RAM[6][64];		//��ʾ���棬ÿ��Bit��Ӧһ���㣬��ɫ��ʾ��1����ʾ�ĵ���ʾ��ɫ�� 0����ʾ�õ���ʾ��ɫ��
static unsigned char Lcd_RAM_x;
static unsigned char Lcd_RAM_y;
static unsigned char g_font;

unsigned char	lcd_refresh_disable;

#define RST_LOW()		do{\
	GPIOA->BRR = GPIO_Pin_8;\
	}while(0)

#define RST_HIGH()		do{\
	GPIOA->BSRR = GPIO_Pin_8;\
	}while(0)

#define CS_LOW()		do{\
	GPIOC->BRR = GPIO_Pin_9;\
	}while(0)

#define CS_HIGH()		do{\
	GPIOC->BSRR = GPIO_Pin_9;\
	}while(0)

#define A0_LOW()		do{\
	GPIOC->BRR = GPIO_Pin_8;\
	}while(0)

#define A0_HIGH()		do{\
	GPIOC->BSRR = GPIO_Pin_8;\
	}while(0)

#define SCK_LOW()		do{\
	GPIOC->BRR = GPIO_Pin_7;\
	}while(0)

#define SCK_HIGH()		do{\
	GPIOC->BSRR = GPIO_Pin_7;\
	}while(0)

#define SDA_LOW()		do{\
	GPIOC->BRR = GPIO_Pin_6;\
	}while(0)

#define SDA_HIGH()		do{\
	GPIOC->BSRR = GPIO_Pin_6;\
	}while(0)

//��ʼ��LCD��IO
static void Lcd_port_init(void)
{
	GPIO_InitTypeDef							GPIO_InitStructure;
	//LED_BLUE	-- PB.12
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA| RCC_APB2Periph_GPIOB | RCC_APB2Periph_GPIOC, ENABLE);

	//LCD_DATA -- PC.6  LCD_SCK -- PC.7  LCD_C/D -- PC.8  LCD_CS -- PC.9
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_6 | GPIO_Pin_7 | GPIO_Pin_8 | GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_50MHz;
	GPIO_Init(GPIOC, &GPIO_InitStructure);

	//LCD_Reset -- PA.8
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_8;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

	//LCD_BACKLight -- PB.12
	GPIO_InitStructure.GPIO_Pin				= GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Mode			= GPIO_Mode_Out_PP;
	GPIO_InitStructure.GPIO_Speed			= GPIO_Speed_10MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

//����V0�Եص�ѹΪ6.4~6.5V
 /*�ڶ��׶Աȶȷ���
     write(0x23,0);
    write(0x81,0);  //Electronic Volume Mode Set
     write(0x13,0);  //Electronic Volume Register Set (�ı��ֵ�ɵ��ڶԱȶȣ����ڷ�Χ0x00~0x3F��
 */
void write(unsigned char data,unsigned char A0)
{
	int i;
	CS_LOW();
	if (A0)
	{
		A0_HIGH();
	}
	else
	{
		A0_LOW();
	}
	for (i = 0; i < 8;i ++)
	{
		if (data&0x80)
		{
			SDA_HIGH();
		}
		else
		{
			SDA_LOW();
		}

		SCK_LOW();

		data<<=1;
		SCK_HIGH();
	}
	CS_HIGH();
}

void write_ext(unsigned char data,unsigned char A0)
{
	int i;
	CS_LOW();
	A0_HIGH();
	for (i = 0; i < 8;i ++)
	{
		if (data&0x80)
		{
			//SDA_HIGH();
			SDA_LOW();
		}
		else
		{
			SDA_HIGH();
		}

		SCK_LOW();

		data<<=1;
		SCK_HIGH();
	}
	CS_HIGH();
}


void Lcd_init(void) //write�������ڶ�����0��ʾָ�1��ʾ����
{
	Lcd_port_init();
	LCD_BACKLIGHT_ON();

	RST_LOW();
	delay_us(20);  //20us
	RST_HIGH();
	delay_us(200);  //200us

	write(0xe2,0);  //Reset
	delay_us(200);   //200us

	write(0xA3,0);  //set 1/6 bias
	write(0xA0,0);  //seg0~seg131
	write(0xc8,0);  //com63~com0

	write(0xa4,0);  //display normal
	write(0x40,0);  //display start line set

	write(0xf8,0);    //booster
	write(0x00,0);    //5xVDD

	// write(0x22,0);  //set (1+Rb/Ra) 100:5.0 ; 111: 6.4
	write(0x23,0);
	// delay(20);  //2ms
	write(0x81,0);  //Electronic Volume Mode Set
	// write(0x19,0);  //Electronic Volume Register Set (�ı��ֵ�ɵ��ڶԱȶȣ����ڷ�Χ0x00~0x3F��
	write(0x13,0);
	// delay(10);   //1ms

	write(0x2c,0);  //power control set
	delay_us(100);   //100us
	write(0x2e,0);  //power control set
	delay_us(100);   //100us
	write(0x2F,0);  //power control set
	delay_us(100);   //100us

	//   delay(50);   //50us
	write(0xaf,0); //Dispaly On
	delay_us(50);   //50us

	Lcd_clear(1);
	Lcd_setfont(FONT_12x6);
}

//c: 0 -- balck   else: white
void Lcd_clear(unsigned char c)
{
	int i=0;
	unsigned char cnt, cnt1,tmp;
	write(0x40,0);      // Set Display Start Line

	tmp = (c==0)?0x00:0xff;
	memset(Lcd_RAM,tmp,64*6);
	for(cnt = 0; cnt < 6; cnt ++)
	{
		write(0xb0 + cnt,0);
		write(0x10,0);
		write(0x00,0);

		for(cnt1 = 0; cnt1 < 64; cnt1 ++)
		{
			write(tmp,1);
			i++;
		}
		//delay(100);

	}
	//delay(800);
}



void set_cursor(unsigned char x,unsigned char y)
{
	Lcd_RAM_x = x;
	Lcd_RAM_y = y/8;
	write(0xb0+(y/8),0);//set page address
	write(0x10+x/16,0);//Set Higer Column Address
	write(x%16,0); //Set Lower Column Address
}


/**
* @brief     ��ָ�����껭��
* @author joe
* @param[in] unsigned short x         ������
* @param[in] unsigned short y         ������
* @param[in] unsigned char  color      ��ɫ   1����ɫ   0����ɫ
* @param[out]  none
* @return  none
* @note  
*     ���÷�����Lcd_SetPoint(10,10,1);
*/
inline void Lcd_SetPoint(unsigned short x,unsigned short y,unsigned short color)
{
	unsigned char offset;

	if ((x>=64)||(y>=48))
	{
		return;
	}
	set_cursor(x,y);
	offset = y%8;		//Ҫ�����ĵ�λ�ڸ��ֽڵĵڼ���bit
	if(color)
	{
		Lcd_RAM[Lcd_RAM_y][Lcd_RAM_x] |= (0x01 << offset);
	}
	else
	{
		Lcd_RAM[Lcd_RAM_y][Lcd_RAM_x] &= ~(0x01 << offset);
	}

	write(Lcd_RAM[Lcd_RAM_y][Lcd_RAM_x],1);
}

//ֻ�Ǹı��Դ������ֵ
inline void Lcd_SetRAMPoint(unsigned short x,unsigned short y,unsigned short color)
{
	unsigned char offset;

	if ((x>=64)||(y>=48))
	{
		return;
	}
	set_cursor(x,y);
	offset = y%8;		//Ҫ�����ĵ�λ�ڸ��ֽڵĵڼ���bit
	if(color)
	{
		Lcd_RAM[Lcd_RAM_y][Lcd_RAM_x] |= (0x01 << offset);
	}
	else
	{
		Lcd_RAM[Lcd_RAM_y][Lcd_RAM_x] &= ~(0x01 << offset);
	}
}

/**
* @brief ��lcd���һС����ģ
*/
void Lcd_PutCharLine(unsigned short x, unsigned short y, unsigned char d, unsigned short charColor) 
{
	int				i;

	if (charColor)
	{
		for(i=0; i<8; i++)
		{
			if(d & 0x80)
				Lcd_SetPoint(x, y++, 1);		// �ַ���ɫ
			else
				Lcd_SetPoint(x, y++, 0);
			d <<= 1;
		}
	}
	else
	{
		for(i=0; i<8; i++)
		{
			if(d & 0x80)
				Lcd_SetPoint(x, y++, 0);		// �ַ���ɫ
			else
				Lcd_SetPoint(x, y++, 1);
			d <<= 1;
		}
	}

}

/**
* @brief ��lcd���һС����ģ
*/
void Lcd_PutCharLine_ext(unsigned short x, unsigned short y, unsigned char d, unsigned short charColor) 
{
	int				i;

	if (charColor)
	{
		for(i=0; i<8; i++)
		{
			if(d & 0x80)
				Lcd_SetPoint(x++, y, 1);		// �ַ���ɫ
			else
				Lcd_SetPoint(x++, y, 0);
			d <<= 1;
		}
	}
	else
	{
		for(i=0; i<8; i++)
		{
			if(d & 0x80)
				Lcd_SetPoint(x++, y, 0);		// �ַ���ɫ
			else
				Lcd_SetPoint(x++, y, 1);
			d <<= 1;
		}
	}

}
#if 0
/**
* @brief ��ˮƽ��
* @note x:����һΪ4�ı���	w:����һΪ4�ı���
*/
void Lcd_DrawLineH(int x, int y, int w,unsigned short color)
{

	unsigned char width;
	unsigned char i;
	width = 64 - x;
	if (width < w)
	{
		w = width;
	}

	for(i = 0; i < w;i++)
	{
		if (color == 0)
		{
			Lcd_SetPoint(x+i,y,0);
		}
		else
		{
			Lcd_SetPoint(x+i,y,1);
		}
	}
}

/**
* @brief ����ֱ��
*/
void Lcd_DrawLineV(int x, int y, int h,unsigned short color)
{
	unsigned char height;
	unsigned char i;
	height = 48 - y;
	if (height < h)
	{
		h = height;
	}

	for(i = 0; i < h;i++)
	{
		if (color == 0)
		{
			Lcd_SetPoint(x,y+i,0);
		}
		else
		{
			Lcd_SetPoint(x,y+i,1);
		}
	}
}

/**
* @brief ���һ��Ӣ���ַ�
*/
static void Lcd_PutChar16_8(unsigned char x,unsigned char y,unsigned char c)
{
	int								i;
	unsigned char		*pData;
	unsigned char		lcd_x, lcd_y;

	lcd_x							= x;
	lcd_y							= y;

	if (c >= 0x20)
	{
		pData			= (unsigned char *)(fontb_en + 16 * (c-0x20));	//�ڿɼ��ַ�֮ǰ�������������ַ�
	}
	else
	{
		return;
	}

	for(i=0; i<8; i++, lcd_x++)
	{
		Lcd_PutCharLine(lcd_x, lcd_y, *pData++, 0);
		Lcd_PutCharLine(lcd_x, lcd_y+8, *pData++, 0);
	}
}
#endif

/**
* @brief ���һ��Ӣ���ַ�
*/
static void Lcd_PutChar12_6(unsigned char x,unsigned char y,unsigned char c)
{
	int								i;
	unsigned char		*pData;
	unsigned char		lcd_x, lcd_y;

	lcd_x							= x;
	lcd_y							= y;

	if (c >= 0x20)
	{
		pData			= (unsigned char *)(AsciiLib_6x12 + 12 * (c-0x20));	//�ڿɼ��ַ�֮ǰ�������������ַ�
	}
	else
	{
		return;
	}

	for(i=0; i<12; i++, lcd_y++)
	{
		Lcd_PutCharLine_ext(lcd_x, lcd_y, *pData++, 0);
	}
}

void Lcd_setfont(unsigned char font_size)
{
	if (font_size == FONT_16x8 || font_size == FONT_12x6)
	{
		g_font = font_size;
	}
}

/**
* @brief �������
*/
void Lcd_TextOut(int x, int y, unsigned char *pText)
{
	unsigned char					*pChar = pText;

	//if (g_font == FONT_16x8)
	//{
	//	if ((x>56)||(y>32))
	//	{
	//		return;
	//	}

	//	while(*pChar)
	//	{
	//		if( *pChar >= 0x20 && *pChar <= 0x80)
	//		{	// ASCII
	//			Lcd_PutChar16_8(x, y, *pChar);
	//			x		+= 8;
	//			if (x>56)
	//			{
	//				break;
	//			}
	//			pChar++;
	//		}
	//	}
	//}
	//else
	{
		if ((x>58)||(y>36))
		{
			return;
		}

		while(*pChar)
		{
			if( *pChar >= 0x20 && *pChar <= 0x80)
			{	// ASCII
				Lcd_PutChar12_6(x, y, *pChar);
				x		+= 6;
				if (x>58)
				{
					break;
				}
				pChar++;
			}
		}
	}
	
}


//LCD�ı�����˸
void Lcd_blink(unsigned int cnt,unsigned int period_ms)
{
	unsigned int i;
	for (i = 0; i < cnt;i++)
	{
		LCD_BACKLIGHT_ON();
		delay_ms(period_ms);
		LCD_BACKLIGHT_OFF();
		delay_ms(period_ms);
	}
}
#if 0
static signed char lcd_flash_cnt;
#define LCD_SLOW_FLASH_SPEED    4
#define LCD_FAST_FLASH_SPEED    60
extern unsigned char BT_current_pin[][5];

void Lcd_disp_BT_info(void)
{
	unsigned char str[11];

	//Lcd_TextOut(0,6,"Name PIN S");
#if(BT_MODULE_CONFIG & USE_BT1_MODULE)
	if (BT1_CONNECT)
	{
		sprintf(str,"HJ1_%s *",&BT_mac[BT1_MODULE][8]);
	}
	else
	{
		sprintf(str,"HJ1_%s  ",&BT_mac[BT1_MODULE][8]);
	}
	Lcd_TextOut(0,6,str);
#endif
	
#if(BT_MODULE_CONFIG & USE_BT2_MODULE)
	if (BT2_CONNECT)
	{
		sprintf(str,"HJ2_%s *",&BT_mac[BT2_MODULE][8]);
	}
	else
	{
		sprintf(str,"HJ2_%s  ",&BT_mac[BT2_MODULE][8]);
	}
	Lcd_TextOut(0,18,str);
#endif
	sprintf(str,"PIN:%s",BT_current_pin[BT1_MODULE]);
	Lcd_TextOut(0,30,str);
	//Lcd_DrawLineH(0,18,64,0);
	//Lcd_DrawLineV(27,0,48,0);
	//Lcd_DrawLineH(0,31,64,0);
	//Lcd_DrawLineV(51,0,48,0);
	//Lcd_DrawLineH(0,45,64,0);
}

void Lcd_Refresh(void)
{
	if (lcd_refresh_disable)
	{
		return;
	}
	if(TPPrinterReady() == 0)	// ��ӡ��δ����
	{
		if(TPPaperReady() == 0 )//ȱֽ
		{
			Lcd_dispBMP(PIC_NOPAPER);
			if(lcd_flash_cnt > 0)
			{
				if((--lcd_flash_cnt) == 0)
				{
					LCD_BACKLIGHT_OFF();
					lcd_flash_cnt = -LCD_FAST_FLASH_SPEED;
				}
			}
			else if(lcd_flash_cnt < 0)
			{
				if((++lcd_flash_cnt) == 0)
				{
					LCD_BACKLIGHT_ON();
					lcd_flash_cnt = LCD_FAST_FLASH_SPEED;
				}
			}
			else
			{
				LCD_BACKLIGHT_ON();
				lcd_flash_cnt = LCD_FAST_FLASH_SPEED;
			}
		}
		else
		{
			LCD_BACKLIGHT_ON();
			Lcd_disp_BT_info();
		}
	}
	else	// ��ӡ������
	{
		LCD_BACKLIGHT_ON();
		lcd_flash_cnt = 0;
		Lcd_disp_BT_info();
	}
}
#endif

static unsigned int step = 0;
void Lcd_process_bar_step(void)
{
	unsigned char str[6];
	step++;
	memset(str,' ',6);str[6] = 0;
	memcpy(str,".....",(step/4)%5);
	Lcd_TextOut(17,24,str);
}
#endif