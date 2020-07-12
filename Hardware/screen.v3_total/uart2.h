#ifndef __UART2__H_
#define __UART2__H_
#include "driverlib.h"

void uart2_init(void);

extern uint8_t USART_RX_BUF[64];       //接收缓冲,最大64个字节.
extern uint8_t USART_RX_NUM;
extern bool flag;     


#endif

