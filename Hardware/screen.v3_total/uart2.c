//#include "uart2.h"
//#include <stdio.h>
//#include <stdint.h>
//#include <stdbool.h>



//uint8_t USART_RX_BUF[64];      
//uint8_t USART_RX_NUM=0; 
//bool flag = true;

//const eUSCI_UART_Config uartConfig =
//{
//				EUSCI_A_UART_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
//				6,                                     // BRDIV = 26
//				8,                                       // UCxBRF = 0
//				32,                                       // UCxBRS = 111
//				EUSCI_A_UART_NO_PARITY,                  // No Parity
//				EUSCI_A_UART_LSB_FIRST,                  // MSB First
//				EUSCI_A_UART_ONE_STOP_BIT,               // One stop bit
//				EUSCI_A_UART_MODE,                       // UART mode
//				EUSCI_A_UART_OVERSAMPLING_BAUDRATE_GENERATION  // Oversampling
//};



//int fputc(int ch, FILE *f)
//{    
////	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);
////	while(status & UCTXIE);
//  MAP_UART_transmitData(EUSCI_A0_BASE, ch);   
//	return ch;
//}



//void uart2_init(void)
//{
//	//GPIO init : Selecting P3.2 and P3.3 in UART mode
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
//						GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);	
//	MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P3,
//						GPIO_PIN2 | GPIO_PIN3, GPIO_PRIMARY_MODULE_FUNCTION);	
//	
//    /* Setting DCO to 48MHz (upping Vcore) */
////    MAP_PCM_setCoreVoltageLevel(PCM_VCORE1);
////    CS_setDCOCenteredFrequency(CS_DCO_FREQUENCY_48);
//	
//		/* Configuring UART Module */
//		MAP_UART_initModule(EUSCI_A0_BASE, &uartConfig);
//    MAP_UART_initModule(EUSCI_A2_BASE, &uartConfig);

//		
//		
//    /* Enable UART module */
//		MAP_UART_enableModule(EUSCI_A0_BASE);
//    MAP_UART_enableModule(EUSCI_A2_BASE);

//    /* Enabling interrupts */
//		MAP_UART_enableInterrupt(EUSCI_A0_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//    MAP_UART_enableInterrupt(EUSCI_A2_BASE, EUSCI_A_UART_RECEIVE_INTERRUPT);
//		MAP_Interrupt_enableInterrupt(INT_EUSCIA0);
//    MAP_Interrupt_enableInterrupt(INT_EUSCIA2);
////    MAP_Interrupt_enableSleepOnIsrExit();
//	
//}

//void EUSCIA0_IRQHandler(void)
//{
//	uint8_t Res;
//	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A0_BASE);

//	MAP_UART_clearInterruptFlag(EUSCI_A0_BASE, status);

//	if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) 
//	{
//			Res = MAP_UART_receiveData(EUSCI_A0_BASE);
////			MAP_UART_transmitData(EUSCI_A0_BASE, Res);
//		
//			USART_RX_BUF[USART_RX_NUM]=Res; 
//			USART_RX_NUM++;
//			if(Res == '\r')
//			{
////				USART_RX_NUM= 0 ;
//					flag = false;
//			}
//		
//			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

//	}
//	
//}

////void EUSCIA2_IRQHandler(void)
////{
////	uint8_t Res;
////	uint32_t status = MAP_UART_getEnabledInterruptStatus(EUSCI_A2_BASE);

////	MAP_UART_clearInterruptFlag(EUSCI_A2_BASE, status);

////	if(status & EUSCI_A_UART_RECEIVE_INTERRUPT_FLAG) 
////	{
////			
////		
////			Res = MAP_UART_receiveData(EUSCI_A2_BASE);
////		MAP_UART_transmitData(EUSCI_A0_BASE,Res);
////			USART_RX_BUF[USART_RX_NUM]=Res ; 
////			USART_RX_NUM++;		
////			if((USART_RX_NUM>63) || (Res == '#'))
////			{
////				USART_RX_NUM= 0 ;
//////				printf("receive a array");
////				
////			}
////		
////			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P1, GPIO_PIN0);

////	}
////	
////}




