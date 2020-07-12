#include "uart1.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h> 
#include "queue.h"

uint8_t USART_RX_BUF[1000];     
char USART_RX_BUF_ADDPRE[200];
uint8_t USART_RX_NUM=0;  
bool flag_json = false; /*Determines whether the json message is received*/

const eUSCI_UART_Config uartConfig =
{
    EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    6,                                     // BRDIV = 26
    8,                                       // UCxBRF = 0
    32,                                       // UCxBRS = 111
    EUSCI_A_UART_NO_PARITY,                  // No Parity
    EUSCI_A_UART_LSB_FIRST,                  // MSB First
    EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
    EUSCI_A_UART_MODE,                       // UART mode
    EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
};


int fputc(int ch, FILE *f)
{      
    MAP_UART_transmitData(EUSCI_A0_BASE, ch);   
    return ch;
}

//bool Send_Data(char *str,uint16_t length){
//	//更新全局
//	strcpy(USART_RX_BUF_ADDPRE,str);
//	for(int i = 0;i<length; i++){
//		MAP_UART_transmitData(EUSCI_A2_BASE, str[i]);
//	}
//	return true;
//}

bool Send_Response(char *str,uint16_t length){
	//{"protocol":"{\"PowerSwitch\":1,\"sat\":100,\"Brightness\":100,\"RGBColor\":{\"Red\":255,\"Green\":255,\"Blue\":255,\"hsl\":[322,100,100]}}"}
	//{"protocol":"{\"switch1\":1,\"switch2\":0}"}
	//{"protocol":"{\"Temperature\":29.24,\"RelativeHumidity\":58.01}"}
	//{"protocol":"{\"data\":\"aaa\"}"}
	for(int i = 0;i<length; i++){
		MAP_UART_transmitData(EUSCI_A2_BASE, str[i]);
		//printf("===%c\n",str[i]);
	}
	///cy
	printf("\n===send Response:\n");
	for(int i=0;i<length;i++){
		printf("%c",str[i]);
	}
	printf("\n???????????????========length,%d\n",length);
	///
	return true;
}

void uart1_init(void)
{
    //GPIO init : Selecting P3.2 and P3.3 in UART mode
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);	
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);	
	
    /* Setting DCO to 48MHz (upping Vcore) */
    //MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    //CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
	
    /* Configuring UART Module */
    MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);

    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_BASE);
    MAP_UART_enableModule(EUSCI_A2_BASE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
//    MAP_Interrupt_enableSleepOnIsrExit();
}

void EUSCIA0_IRQHandler(void)
{
	uint8_t Res;
	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

//	MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

//	if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) 
//	{
//			Res = MAP_UART_receiveData(EUSCI_A0_BASE);
//		
//			//将A0收到的内容传给A2
//			MAP_UART_transmitData(EUSCI_A2_BASE,Res);
//		
//			USART_RX_BUF[USART_RX_NUM]=Res; 
//			USART_RX_NUM++;
//			//判断指令是否结束
//			if(Res == '\r')
//			{
//				USART_RX_NUM= 0 ;
//				//uart_flag = true;
//			}
//		
//			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

//	}
	
}

void EUSCIA2_IRQHandler(void)
{
    uint8_t Res;
    uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

    MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

    if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) 
    {
        Res = MAP_UART_receiveData(EUSCI_A2_BASE);
		//	printf("=====receive data from Iot\n");
			//printf("|%c|\n",Res);
	//		MAP_UART_transmitData(EUSCI_A0_BASE, 33); 
       MAP_UART_transmitData(EUSCI_A0_BASE, Res);  
        EnQueue(&q,Res);
        
//        USART_RX_BUF[USART_RX_NUM++]=Res;
    }
}


