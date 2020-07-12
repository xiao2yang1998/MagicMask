/* DriverLib Includes */
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

/* Standard Includes */
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include "uart1.h"
#include "userButton.h"

bool user_button_flag = false;

void userButton_restore(void)
{
//    /* Configuring P4.6 as input */
//		MAP_GPIO_setAsOutputPin(GPIO_PORT_P1, GPIO_PIN0);

   	/* Configuring P4.6 as an input and enabling interrupts */
    MAP_GPIO_setAsInputPinWithPullUpResistor(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, GPIO_PIN6);
    MAP_GPIO_enableInterrupt(GPIO_PORT_P4, GPIO_PIN6);
    MAP_Interrupt_enableInterrupt(INT_PORT4);

    MAP_SysCtl_enableSRAMBankRetention(SYSCTL_SRAM_BANK1);
    MAP_Interrupt_enableMaster();   
		
}

/* GPIO ISR */
void PORT4_IRQHandler(void)
{
    uint32_t status;

    status = MAP_GPIO_getEnabledInterruptStatus(GPIO_PORT_P4);
    MAP_GPIO_clearInterruptFlag(GPIO_PORT_P4, status);

    /* Toggling the output on the LED */
    if(status & GPIO_PIN6)
    {
//			MAP_GPIO_setOutputHighOnPin(GPIO_PORT_P4,GPIO_PIN6);
		user_button_flag = true;
//				Send_AT("AT+REBOOT\r",11);
//				Send_AT("AT+AT\r",3);
//				Send_AT("AT+UARTE=OFF\r",11);
//				Send_AT("AT+AT\r",3);
    }

}
