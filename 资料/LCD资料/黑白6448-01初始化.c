 //HEM-IC2

void initial(void) //write函数，第二参数0表示指令，1表示数据
    {

     RST=0;
     delay(20);  //20us
     RST=1;
     delay(200);  //200us
   
     write(0xe2,0);  //Reset
     delay(200);   //200us

     write(0xA3,0);  //set 1/6 bias
     write(0xA0,0);  //seg0~seg131
     write(0xc8,0);  //com63~com0

     write(0xa4,0);  //display normal
     write(0x40,0);  //display start line set

     write(0xf8,0);    //booster
     write(0x00,0);    //5xVDD

     write(0x22,0);  //set (1+Rb/Ra) 100:5.0 ; 111: 6.4
    // delay(20);  //2ms
     write(0x81,0);  //Electronic Volume Mode Set
     write(0x19,0);  //Electronic Volume Register Set (改变此值可调节对比度，调节范围0x00~0x3F）
    // delay(10);   //1ms

     write(0x2c,0);  //power control set
     delay(100);   //100us
     write(0x2e,0);  //power control set
     delay(100);   //100us
     write(0x2F,0);  //power control set
     delay(100);   //100us
     
   //  display(0x00,0x00);

  //   delay(50);   //50us
     write(0xaf,0); //Dispaly On
     delay(50);   //50us
     }


void DispBmp1(void)//normal
{
     int i=0;
   uchar cnt, cnt1;
   write(0x40,0);      // Set Display Start Line

   for(cnt = 0; cnt < 6; cnt ++)
     {
         write(0xb0 + cnt,0);
         write(0x10,0);
         write(0x00,0);

         for(cnt1 = 0; cnt1 < 64; cnt1 ++)
           {write(AA[i],1);
           i++;}
        //delay(100);

      }
//delay(800);
}
