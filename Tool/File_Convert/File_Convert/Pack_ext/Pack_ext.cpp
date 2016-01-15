// Pack_ext.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "stdlib.h"
#include "string.h"
#include "conio.h"

#define TAG_START		0x01
#define TAG_BIN_DATA	0x02
#define TAG_FNT_DATA	0x03
#define TAG_PIC_DATA	0x04
#define TAG_END			0x05

//处理的文件类型
#define FILE_TYPE_BIN	2
#define FILE_TYPE_FONT	3
#define FILE_TYPE_PIC	4
/* 
*********************************************************************** 
* 函数： TCHAR2Char 
* 描述：将TCHAR* 转换为 char* 
* 日期：
*********************************************************************** 
*/ 
char* TCHAR2char(TCHAR* tchStr) 
{ 
	int iLen = 2*wcslen(tchStr);//CString,TCHAR汉字算一个字符，因此不用普通计算长度 
	char* chRtn = new char[iLen+1];
	wcstombs(chRtn,tchStr,iLen+1);//转换成功返回为非负值 
	return chRtn; 
} 

unsigned char HexToAscii(unsigned char hex)
{
	if(hex<0x0a)
		return (0x30+hex);
	else
		return (0x37+hex);
}

int _tmain(int argc, _TCHAR* argv[])
{
	//获取当前运行的路径
	//char pBuf[260];                                 //存放路径的变量     

	char *src_bin_file;
	char *target_bin_file;
	char write_buf[140];
	//char read_buf[60];
	int	offset,write_len,check_sum;
	char file_type;
	//char *oem_name;
	//char *version;
	//char *date;
	FILE *fp_src;
	FILE *fp_target;

	if (argc == 1)
	{
		exit(0);
	}

	if (argc == 4)
	{
		src_bin_file = TCHAR2char(argv[1]);
		target_bin_file = TCHAR2char(argv[2]);
		file_type = *argv[3];
		file_type -= 0x30;
		if ((file_type != FILE_TYPE_BIN)&&(file_type != FILE_TYPE_FONT)&&(file_type != FILE_TYPE_PIC))
		{
			printf("\n [the 3rd param [file_type] should be:1--BIN  2--FONT  3--PIC]\n");
			getch();
			exit(1);
		}
	}
	else
	{
		printf("\n Pack_ext [src_file] [target_file] [file_type:1--BIN  2--FONT  3--PIC]\n");
		getch();
		exit(1);
	}

	
	
	fp_src =  fopen((const char*)src_bin_file,"rb");
	if (fp_src == NULL)
	{
		printf("\n open source file: %s fail!\n",src_bin_file);
		getch();
		exit(1);
	}

	fp_target =  fopen((const char*)target_bin_file,"wb+");
	if (fp_src == NULL)
	{
		printf("\n open target file: %s fail!\n",target_bin_file);
		getch();
		exit(1);
	}

	fseek(fp_target, 0, SEEK_SET);

	check_sum = ~((TAG_START+0x01)%256)+1;
	write_buf[0]=':';
	write_buf[1]=HexToAscii((TAG_START>>4)&0x0f);
	write_buf[2]=HexToAscii(TAG_START&0x0f);
	write_buf[3]=0x30;
	write_buf[4]=0x30;
	//write_buf[5]=0x30;
	//write_buf[6]=0x30;
	write_buf[5]=HexToAscii((file_type>>4)&0x0f);
	write_buf[6]=HexToAscii(file_type&0x0f);		//file type
	write_buf[7]=0x30;
	write_buf[8]=0x31;
	write_buf[9]=HexToAscii((check_sum>>4)&0x0f);
	write_buf[10]=HexToAscii(check_sum&0x0f);
	write_buf[11]=0x0D;
	write_buf[12]=0x0A;

	write_len = fwrite((const char*)write_buf,13,1,fp_target);
	if (write_len != 1)
	{
		fclose(fp_target);
		printf("\n write target file: %s fail!\n",target_bin_file);
		getch();
		exit(1);
	}
	fseek(fp_target, 13, SEEK_SET);

	write_buf[1]=HexToAscii((file_type>>4)&0x0f);
	write_buf[2]=HexToAscii(file_type&0x0f);		//TAG
	check_sum = file_type;

	fseek(fp_src, 0, SEEK_END);
	int len = ftell(fp_src);  
	char* read_buf=new char[len];  
	memset(read_buf,0,len);
	fseek(fp_src, 0, SEEK_SET);  
	int iRead=fread_s(read_buf,len,1,len,fp_src);
	fclose(fp_src);
	if (iRead != len)
	{
		printf("\n read src file: %s fail!\n",src_bin_file);
		getch();
		exit(1);
	}
	offset = 0;
	while((offset+48) < len)
	{
		write_buf[3] = HexToAscii((48>>4)&0x0f);	//LEN
		write_buf[4] = HexToAscii(48&0x0f);
		check_sum += 48;
		for(int i = 0; i < 48; i++)
		{
			write_buf[5+2*i] = HexToAscii((read_buf[offset+i]>>4)&0x0f);
			write_buf[5+2*i+1] =HexToAscii(read_buf[offset+i]&0x0f);
			check_sum += read_buf[offset+i];
		}
		check_sum %= 256;
		check_sum = ~check_sum;
		check_sum += 1;
		write_buf[5+2*48]=HexToAscii((check_sum>>4)&0x0f);
		write_buf[6+2*48]=HexToAscii(check_sum&0x0f);
		write_buf[7+2*48] = 0x0D;
		write_buf[8+2*48] = 0x0A;

		write_len = fwrite((const char*)write_buf,9+2*48,1,fp_target);
		if (write_len != 1)
		{
			fclose(fp_target);
			printf("\n write target file: %s fail!\n",target_bin_file);
			getch();
			exit(1);
		}
		//fseek(fp_target, 7+2*64, SEEK_CUR);
		offset += 48;
		check_sum = file_type;
	}

	if (offset < len)
	{
		len -= offset;
		write_buf[3] = HexToAscii((len>>4)&0x0f);	//LEN
		write_buf[4] = HexToAscii(len&0x0f);
		check_sum += len;
		for(int i = 0; i < len; i++)
		{
			write_buf[5+2*i] = HexToAscii((read_buf[offset+i]>>4)&0x0f);
			write_buf[5+2*i+1] =HexToAscii(read_buf[offset+i]&0x0f);
			check_sum += read_buf[offset+i];
		}
		check_sum %= 256;
		check_sum = ~check_sum;
		check_sum += 1;
		write_buf[5+2*len]=HexToAscii((check_sum>>4)&0x0f);
		write_buf[6+2*len]=HexToAscii(check_sum&0x0f);
		write_buf[7+2*len] = 0x0D;
		write_buf[8+2*len] = 0x0A;

		write_len = fwrite((const char*)write_buf,9+2*len,1,fp_target);
		if (write_len != 1)
		{
			fclose(fp_target);
			printf("\n write target file: %s fail!\n",target_bin_file);
			getch();
			exit(1);
		}
	}

	check_sum = TAG_END+0x01;
	check_sum %= 256;
	check_sum = ~check_sum;
	check_sum += 1;
	write_buf[0]=':';
	write_buf[1]=HexToAscii((TAG_END>>4)&0x0f);
	write_buf[2]=HexToAscii(TAG_END&0x0f);
	write_buf[3]=0x30;
	write_buf[4]=0x30;
	write_buf[5]=HexToAscii((file_type>>4)&0x0f);
	write_buf[6]=HexToAscii(file_type&0x0f);		//file type
	write_buf[7]=0x30;
	write_buf[8]=0x31;
	write_buf[9]=HexToAscii((check_sum>>4)&0x0f);
	write_buf[10]=HexToAscii(check_sum&0x0f);
	write_buf[11]=0x0D;
	write_buf[12]=0x0A;
	write_len = fwrite((const char*)write_buf,13,1,fp_target);
	if (write_len != 1)
	{
		fclose(fp_target);
		printf("\n write target file: %s fail!\n",target_bin_file);
		getch();
		exit(1);
	}

	fclose(fp_target);

	return 0;
}

