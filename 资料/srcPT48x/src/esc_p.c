#include "includes.h"
//======================================================================================================
//======================================================================================================
//======================================================================================================
//#define NULL	(0x00)
#define SOH		(0x01)
#define STX		(0x02)
#define ETX		(0x03)
#define EOT		(0x04)
#define ENQ		(0x05)
#define ACK		(0x06)
#define BEL		(0x07)
#define BS		(0x08)
#define HT		(0x09)
#define LF		(0x0a)
#define VT		(0x0b)
#define FF		(0x0c)
#define CR		(0x0d)
#define SO		(0x0e)
#define SI		(0x0f)
#define DLE		(0x10)
#define DC1		(0x11)
#define DC2		(0x12)
#define DC3		(0x13)
#define DC4		(0x14)
#define NAK		(0x15)
#define SYN		(0x16)
#define ETB		(0x17)
#define CAN		(0x18)
#define EM		(0x19)
#define SUB		(0x1a)
#define ESC		(0x1b)
#define FS		(0x1c)
#define GS		(0x1d)
#define RS		(0x1e)
#define US		(0x1f)
#define SP		(0x20)

ESC_P_STS_T  esc_sts;
extern void esc_p_init(void)
{
	uint8_t i;
//----chang
    esc_sts.international_character_set = 0;    // english
    esc_sts.character_code_page = para.character_code_page;

	esc_sts.prt_on = 0;
	esc_sts.larger = 0;
    #ifdef ASCII9X24
    esc_sts.font_en = FONT_B_WIDTH;	// 字体
    #else
    esc_sts.font_en = FONT_A_WIDTH;	// 字体
    #endif
	esc_sts.font_cn = FONT_CN_A_WIDTH;	// 字体
	esc_sts.bold = 0;		// 粗体
	esc_sts.double_strike=0;//重叠打印
	esc_sts.underline = 0;	// 下划线
	esc_sts.revert = 0;		// 反白显示
	esc_sts.rotate = 0;
	esc_sts.start_dot = 0;
    esc_sts.smoothing_mode = 0;	// 平滑模式
	esc_sts.dot_minrow = ARRAY_SIZE(esc_sts.dot[0]);
	memset(esc_sts.dot, 0 ,sizeof(esc_sts.dot));
	for(i=0; i<8; i++)
	{
		esc_sts.tab[i] = 9+8*i;
	}
	esc_sts.linespace = 30;
	esc_sts.charspace = 0;
	esc_sts.align = 0;
	esc_sts.leftspace = 0;
    esc_sts.print_width=LineDot;
    esc_sts.upside_down=0;//倒置
	esc_sts.barcode_height = 50;
	esc_sts.barcode_width = 2;
    esc_sts.barcode_leftspace = 0;
	esc_sts.barcode_char_pos = 0;//不显示
	esc_sts.barcode_font = 0;
	esc_sts.userdefine_char = 0;
    esc_sts.asb_mode=0;

	esc_sts.chinese_mode = 1;
	esc_sts.bitmap_flag = 0;

  if(esc_sts.status4 == 0)
  {
      esc_sts.status4=0x12;
  }

}
extern esc_init(void)
{
	esc_p_init();
}

extern void esc_p(void)
{
	uint8_t cmd;

	switch(cmd=Getchar())
	{
	  case LF:	// line feed
	      PrintCurrentBuffer(0);
		  break;
      case CR:      // carry return
          break;
	  case ESC:		// ESC
		  break;
	  case FS:		// FS
          break;
	  case GS:		// GS
          break;
      case CAN:
		  break;
	  default:
        {
            //----chang
#if !defined(CHINESE_FONT)||defined (CODEPAGE)
			if((cmd >= 0x20) && (cmd <= 0xff))
			{
				GetEnglishFont(cmd);
			}
#else
        if((cmd >= 0x20) && (cmd <= 0x7f))
			{
				GetEnglishFont(cmd);
			}
#if defined(GB2312)
        else if ((cmd >= 0xa1) && (cmd <= 0xfe))
        {
            uint8_t chs[2];
            chs[0] = cmd;
            chs[1] = Getchar();
            if ((chs[1] >= 0xa1) && (chs[1] <= 0xfe)
            {
                GetChineseFont(chs, CHINESE_FONT_GB2312);
            }
            else
            {
                GetEnglishFont('?');
                GetEnglishFont('?');
            }
        }
#elif defined(GBK) || defined(GB18030)
        else if ((cmd >= 0x81) && (cmd <= 0xfe))
        {
            uint8_t chs[4];
            chs[0] = cmd;
            chs[1] = Getchar();
#if defined(GB18030)
            if ((chs[1] >= 0x30) && (chs[1] <= 0x39))
#else
                if (0)
#endif
                {
                    chs[2] = Getchar();
                    chs[3] = Getchar();
                    // GB18030定义的4字节扩展
                    if (((chs[2] >= 0x81) && (chs[2] <= 0xfe)) && ((chs[3] >= 0x30) && (chs[3] <= 0x39)))
                    {
                        GetChineseFont(chs, CHINESE_FONT_GB18030);
                    }
                    else
                    {
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                        GetEnglishFont('?');
                    }
                }
                // GB13000定义的2字节扩展
                else if ((chs[1] >= 0x40) && (chs[1] <= 0xfe) && (chs[1] != 0x7f))
                {
                    GetChineseFont(chs, CHINESE_FONT_GB13000);
                }
                else
                {
                    GetEnglishFont('?');
                    GetEnglishFont('?');
                }
        }
#endif
#endif
	   }
}
//======================================================================================================

