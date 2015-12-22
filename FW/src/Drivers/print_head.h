#ifndef _PRINT_HEAD_H_
#define _PRINT_HEAD_H_

void print_head_init(void);
void print_head_spi_send_data(unsigned char *data,unsigned int len);

#endif