#ifndef __UART0__H_
#define __UART0__H_
#include "driverlib.h"

void uart0_init(void);

extern uint8_t USART_RX_BUF[64];       //���ջ���,���64���ֽ�.
extern uint8_t USART_RX_NUM;     


#endif

