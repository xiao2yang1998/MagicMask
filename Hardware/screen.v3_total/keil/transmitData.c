#include "uart0.h"
#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>



uint8_t USART_RX_BUF[64];       //接收缓冲，最大64个字节
uint8_t USART_RX_NUM=0;     


//重定义fputc函数
int fputc(int ch, FILE *f)
{      
	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_MODULE);
	while(status & UCTXIE); 
    MAP_UART_transmitData(EUSCI_A0_MODULE, ch);   
	return ch;
}



void uart0_init(void)
{
	const eUSCI_UART_Config uartConfig =
	{
					EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
					26,                                     // BRDIV = 26
					0,                                       // UCxBRF = 0
					111,                                       // UCxBRS = 111
					EUSCI_A_UART_NO_PARITY,                  // No Parity
					EUSCI_A_UART_LSB_FIRST,                  // MSB First
					EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
					EUSCI_A_UART_MODE,                       // UART mode
					EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
	};
	
	
	//GPIO init : Selecting P1.2 and P1.3 in UART mode
	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
						GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);	
	
    /* Setting DCO to 48MHz (upping Vcore) */
    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
	
		/* Configuring UART Module */

    MAP_UART_initModule(EUSCI_A0_MODULE, &uartConfig);

		
		
    /* Enable UART module */
    MAP_UART_enableModule(EUSCI_A0_MODULE);

    /* Enabling interrupts */
    MAP_UART_enableInterrupt(EUSCI_A0_MODULE, EUSCI_A_UART_RECEIVE_INTERRUPT);
    MAP_Interrupt_enableInterrupt(INT_EUSCIA0);

	
}


void EUSCIA0_IRQHandler(void)
{
	uint8_t Res;
	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_MODULE);

	MAP_UART_clearInterruptFlag(EUSCI_A0_MODULE, status);

	if(status & EUSCI_A_UART_RECEIVE_INTERRUPT) 
	{
			Res = MAP_UART_receiveData(EUSCI_A0_MODULE);
		
			USART_RX_BUF[USART_RX_NUM]=Res ;
			USART_RX_NUM++;		
			if((USART_RX_NUM>63) || (Res == '#'))
			{
				USART_RX_NUM= 0 ;
			}
		
			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);
//			MAP_Interrupt_disableSleepOnIsrExit();
	}
	
}




