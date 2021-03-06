/*;----------------------------------------------------------
;MODULE     :HEM12864-253B(128X64Pixls)
;AUTHOR     :XQC
;DATE       :2015/7/23
;CONTROLLER :HEM-IC1 (Serial Interface)
;Revision   :VCC=3.3V
;       :Boost=4X
;       :V0-Vss=9.0V (1/64 duty,1/9 bias)
;       :Com63~Com0,Seg0~127
; 大玻璃朝上，fpc朝下，6点视角
;----------------------------------------------------------*/

#include <stdio.h>
#include <ctype.h>
#include <reg52.h>
#include <intrins.h>
#include <string.h>       

#define dataport P1
#define  INTERFACE  2       //0:8080,   1:6800,   2:4SPI,  3:3SPI
#define  tim 400

typedef unsigned char uchar;
typedef unsigned int uint;

//sbit CS=P3^4;
//sbit RST=P3^3;
sbit CS=P3^5;
sbit RST=P3^6;
//sbit RW_WR=P3^6;  //WR0
//sbit E_RD=P3^5;    //WR1
sbit CD=P3^7;     //A0

sbit SCL=P1^0;   //D0
sbit SDA=P1^1;    //D3

uchar bdata Temp_Buff;
sbit    Temp_Buff7 = Temp_Buff^7;
sbit    Temp_Buff6 = Temp_Buff^6;
sbit    Temp_Buff5 = Temp_Buff^5;
sbit    Temp_Buff4 = Temp_Buff^4;
sbit    Temp_Buff3 = Temp_Buff^3;
sbit    Temp_Buff2 = Temp_Buff^2;
sbit    Temp_Buff1 = Temp_Buff^1;
sbit    Temp_Buff0 = Temp_Buff^0;





void delay(uint t)
    {

     while(--t);

     }


void delay1(uint t)
    {
     uint i,j;
     for (i=0;i<t;i++)
     for (j=0;j<100;j++);
     }


void  SPI_SendByte(uchar c)
{
    
    Temp_Buff = c;
    SDA=Temp_Buff7 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff6;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff5 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff4 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff3 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff2 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff1 ;
        SCL=0;
        SCL=1;
    SDA=Temp_Buff0;
        SCL=0;
        SCL=1;
     
}

void write(uchar _data, bit di) //di=0, write command
    {                           //di=1,write data
          #if  INTERFACE==0 //8080  BM[1:0]:HL
          {
         E_RD=1;

     CD=di;
     CS=0;
     dataport=_data;
     RW_WR=0;
     _nop_();
     _nop_();
     RW_WR=1;
     CD=~di;
     CS=1;
         }
    #elif INTERFACE == 1 //6800
    {
            CS  = 0 ;
            CD=di;
            RW_WR= 0;
            _nop_();
            _nop_();
            _nop_();
            _nop_();
                    
            dataport=_data;
            E_RD= 1;
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();
            _nop_();            
            E_RD= 0;
            
            CD=~di;
            CS = 1;

    }
         //4-spi  BM[1:0]:LL
         #elif INTERFACE == 2
         {
        CD = di;
        CS = 0;
        SPI_SendByte(_data);
    //    CD = ~di;
        CS = 1;
         }
        
      #endif    
 }

void init_ads(uchar page)
{
    write(page,0);//set page address
    write(0x10,0);//Set Higer Column Address
    write(0x04,0); //Set Lower Column Address
}

void display(uchar a,uchar b)
    {
      uchar pag=0xb0;
      uint i,j;
     write(0x40,0); //Set Display Start Line
      for(i=0;i<8;i++)   //8 pages
      {
        init_ads(pag);
        for(j=0;j<64;j++)
        {
           write(a,1);
           write(b,1);
        }
        pag++;
      }
      delay1(tim);
     }



unsigned char code HZ[]={
/*--  文字:  华  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
//0x00,0x20,0x20,0x20,0x20,0x20,0x20,0x20,0xFF,0x20,0x20,0x20,0xA0,0x20,0x20,0x00,
//0x00,0x00,0x27,0x61,0x21,0x11,0x11,0xFE,0x08,0x08,0x40,0xC0,0x3F,0x10,0x08,0x04,
/*--  文字:  徐  --*/
/*--  宋体12;  此字体下对应的点阵为：宽x高=16x16   --*/
0x00,0x08,0x9C,0x90,0xA0,0x80,0xFE,0x81,0x92,0xB0,0x88,0x04,0x00,0xFF,0x00,0x80,
0x00,0x08,0x0C,0x18,0x14,0x24,0xC7,0x24,0x14,0x08,0x04,0x44,0xCE,0x23,0x11,0x08,
};



     
void DispHZ(void)//normal
{

   uchar i,cnt, cnt1,page,x0,x1;
   write(0x40,0);      // Set Display Start Line
   page=0xb0;
  for(x0=0;x0<4;x0++){

  for(x1=0;x1<8;x1++){
    i=0;

   for(cnt = 0; cnt < 2; cnt ++)
     {

         write(page + cnt + 2*x0,0);
         write(0x10 + x1,0);
         write(0x04,0);

         for(cnt1 = 0; cnt1 < 16; cnt1 ++)
           {
            write(HZ[i],1);
            i++;
            }
    //    delay1(10);

      }
                    }
                    }
    delay1(tim);
}

unsigned char code AA[]= {
/*--  调入了一幅图像：D:\My Documents\程序调试\zimo\128x64_da.bmp  --*/
/*--  宽度x高度=128x64  --*/
0xFF,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,
0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x80,0xC0,
0x60,0xF8,0xFE,0x06,0x00,0x00,0x80,0xFE,0xFE,0x40,0x60,0x30,0x18,0x18,0x88,0x80,
0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xF0,0x3C,0x3E,0x26,0x20,0xA0,0xA0,0x20,0x20,
0x20,0xA0,0xE0,0x70,0x30,0x20,0x00,0x00,0x00,0x00,0x10,0x10,0x10,0x10,0x18,0xF8,
0xF8,0x0C,0x04,0x04,0x04,0x00,0xFC,0xFC,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x10,
0xF0,0xF0,0x10,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xE0,0x30,0x10,
0x10,0x10,0x30,0xF0,0xF0,0x00,0x00,0x10,0xF0,0xF0,0x10,0x10,0x10,0x30,0xE0,0xC0,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x41,0x41,0x40,
0x40,0x4F,0x4F,0x40,0x41,0x41,0xF9,0xFF,0x47,0x44,0x44,0x44,0x44,0x44,0x67,0x67,
0x44,0x00,0x00,0x00,0x00,0x01,0xC1,0xF0,0x3C,0x1C,0x0C,0x00,0xFF,0xFF,0x00,0x04,
0x0C,0x18,0x70,0xE0,0xC0,0x80,0x00,0x00,0x00,0x02,0x02,0x02,0x02,0x02,0xC2,0xFF,
0x3F,0x02,0x02,0x02,0x02,0x02,0xFF,0xFF,0x02,0x02,0x03,0x03,0x02,0x00,0x00,0x00,
0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x80,0x80,0x00,0x00,0x7F,0xFF,0x80,0x00,0x00,
0x00,0x00,0x00,0xC0,0xC0,0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0xC0,0xFF,
0x3F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x00,0x02,0x03,0x01,0x00,0x00,0x00,0x04,0x04,0x0F,0x0F,0x00,0x00,
0x00,0x00,0x00,0x01,0x01,0x01,0x00,0x00,0x00,0x08,0x08,0x0C,0x06,0x03,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x0F,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x02,
0x03,0x03,0x02,0x02,0x02,0x02,0x03,0x03,0x00,0x00,0x00,0x00,0x01,0x01,0x03,0x02,
0x02,0x02,0x03,0x01,0x00,0x00,0x00,0x02,0x03,0x03,0x02,0x02,0x02,0x03,0x01,0x00,
0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xC0,0xC0,
0x00,0x00,0x00,0x00,0x00,0x80,0xC0,0xC0,0xC0,0xC0,0xC0,0x80,0x00,0x00,0x00,0x80,
0xC0,0xC0,0xC0,0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x00,0x00,0x80,0x80,0xC0,0xC0,0xC0,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
0x80,0xC0,0xC0,0x00,0x00,0x00,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x80,0x80,0x00,
0x00,0x00,0x00,0x00,0x80,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x80,0x00,0x00,0xC0,0xC0,
0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0xC0,0x00,0x80,0xC0,0xC0,0xC0,0xC0,
0xC0,0xC0,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x03,0x03,0x03,0xFF,0xFF,
0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x80,0xC0,0x60,0x31,0x1F,0x0F,0x00,0xC7,0xEF,
0x39,0x18,0x10,0x30,0x79,0xEF,0xC7,0x00,0x04,0x1C,0x38,0xF0,0xF0,0x38,0x1C,0x04,
0x00,0xFC,0xFF,0x13,0x19,0x18,0x18,0x38,0xF0,0xE0,0x00,0x70,0x78,0x6C,0x66,0x63,
0x61,0xFF,0xFF,0x60,0x60,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x01,0x01,0x03,0xFF,
0xFC,0x00,0xFC,0xFF,0x03,0x01,0x00,0x00,0x00,0x00,0x01,0x03,0xFF,0xFC,0x00,0x00,
0x00,0x00,0x00,0xFF,0xFF,0x00,0x00,0x00,0x00,0x00,0x0F,0x1F,0x19,0x30,0x30,0x30,
0x30,0x60,0xE1,0xC0,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0C,0x0C,0x0C,0x0F,0x0F,
0x0C,0x0C,0x0C,0x00,0x00,0x0E,0x0F,0x0D,0x0C,0x0C,0x0C,0x0C,0x0C,0x0C,0x03,0x07,
0x0E,0x0C,0x0C,0x0C,0x0E,0x07,0x03,0x00,0x08,0x0E,0x07,0x03,0x03,0x07,0x0E,0x08,
0x00,0x01,0x07,0x0E,0x0C,0x0C,0x0C,0x0E,0x07,0x03,0x00,0x00,0x00,0x00,0x00,0x00,
0x00,0x0F,0x0F,0x00,0x00,0x00,0x0F,0x0F,0x0C,0x0C,0x0C,0x0C,0x0E,0x06,0x07,0x03,
0x00,0x00,0x00,0x03,0x07,0x0E,0x0C,0x0C,0x0C,0x0C,0x0E,0x07,0x03,0x00,0x00,0x00,
0x00,0x00,0x00,0x0F,0x0F,0x00,0x00,0x00,0x00,0x00,0x06,0x0C,0x0C,0x0C,0x0C,0x0C,
0x0C,0x06,0x07,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0xFF,
0xFF,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,
0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0x80,0xFF,
};


void DispBmp1(void)//normal
{
     int i=0;
   uchar cnt, cnt1;
   write(0x40,0);      // Set Display Start Line

   for(cnt = 0; cnt < 8; cnt ++)
     {
         write(0xb0 + cnt,0);
         write(0x10,0);
         write(0x04,0);

         for(cnt1 = 0; cnt1 < 128; cnt1 ++)
           {write(AA[i],1);
           i++;}
     //   delay(100);

      }
delay1(tim);
}




void initial(void)
    {

     RST=0;
     delay(20);  //20us
     RST=1;
     delay(200);  //200us
   
     write(0xe2,0);  //Reset
     delay(200);   //200us

     write(0xA2,0);  //set 1/9 bias
     write(0xA1,0);  //seg131~seg0
     write(0xc8,0);  //com63~com0

     write(0xa4,0);  //display normal
     write(0x40,0);  //display start line set

     write(0x24,0);  //set (1+Rb/Ra) 100:5.0 ; 111: 6.4   
    // delay(20);  //2ms
     write(0x81,0);  //Electronic Volume Mode Set
     write(0x28,0);  //Electronic Volume Register Set (改变此值可调节对比度，调节范围0x00~0x3F）
    // delay(10);   //1ms

     write(0x2c,0);  //power control set
     delay(100);   //100us
     write(0x2e,0);  //power control set
     delay(100);   //100us
     write(0x2F,0);  //power control set
     delay(100);   //100us
     
     display(0x00,0x00);

     delay(50);   //50us
     write(0xaf,0); //Dispaly On
     delay(50);   //50us
     }

 /*
void call_init(void)
    {


     write(0xe2,0);  //Reset
     delay(50);   //50us

     write(0xA2,0);  //set 1/9 bias
     write(0xA0,0);  //seg0~seg131
     write(0xc8,0);  //com63~com0

    // write(0xa4,0);  //display normal
    // write(0x40,0);  //display start line set

     write(0x24,0);  //set (1+Rb/Ra) 100:5.0 ; 111: 6.4   
    // delay(20);  //2ms
     write(0x81,0);  //Electronic Volume Mode Set
     write(0x24,0);  //Electronic Volume Register Set (改变此值可调节对比度，调节范围0x00~0x3F）
    // delay(10);   //1ms


     write(0x2F,0);  //power control set
     delay(50);   //50us


     write(0xaf,0); //Dispaly On
     delay(50);   //50us
     }
*/

void main(void)
    { 
    
    // initial();
  //   delay1(500);
     while (1)
         {

       initial();
       //  delay1(500);
        display(0xff,0xff);
       // delay1(1000);

     //   call_init();
        display(0x00,0x00);
       // delay1(1000);
    //    call_init();
        display(0xaa,0x55);
      //  delay1(1000);
    //    call_init();
    //    DispHZ();
    //  delay1(1000);
    //    call_init();
        DispBmp1();
    //  delay1(1000);

    }
}


