#ifndef __UART1__H_
#define __UART1__H_
#include "driverlib.h"


//extern uint8_t USART_RX_BUF[1000];       
extern char USART_RX_BUF_ADDPRE[200];
//extern uint8_t USART_RX_NUM;
extern bool flag_json;
//extern bool uart_flag;


void uart1_init(void);
bool Send_Data(char *str,uint16_t length);
bool Send_Response(char *str,uint16_t length);



#endif

