///* DriverLib Includes */
//#include <ti/devices/msp432p4xx/driverlib/driverlib.h>

///* Standard Includes */
//#include <string.h>
//#include <stdlib.h>
//#include <stdint.h>
//#include <stdbool.h>
//#include <stdio.h>
//#include "oled.h"
//#include "tick.h"
//#include "i2c.h"
//#include "sht2x.h"

//Timer_A_PWMConfig pwm_red_Config_1 =
//{
//        TIMER_A_CLOCKSOURCE_SMCLK,		//12M
//        TIMER_A_CLOCKSOURCE_DIVIDER_16,		//12M/16=750000
//        3750,												//3750/750000=5ms
//        TIMER_A_CAPTURECOMPARE_REGISTER_4,
//        TIMER_A_OUTPUTMODE_RESET_SET,
//        3750
//};
//int main(void)
//{
//    MAP_WDT_A_holdTimer();
//		MAP_FPU_enableModule();  
//	
//		//set clock
//    MAP_CS_setDCOFrequency(12000000);
//		MAP_CS_initClockSignal(CS_MCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
//		MAP_CS_initClockSignal(CS_HSMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1);
//		MAP_CS_initClockSignal(CS_SMCLK, CS_DCOCLK_SELECT, CS_CLOCK_DIVIDER_1); //12M
//	
//		MAP_SysTick_enableModule();
//		MAP_SysTick_setPeriod(12000);
//	  MAP_SysTick_enableInterrupt();
//	
//		MAP_Interrupt_enableMaster();
//		
//		MAP_GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P2, GPIO_PIN4 + GPIO_PIN6 + GPIO_PIN7,GPIO_PRIMARY_MODULE_FUNCTION);
//		MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_red_Config_1);
//	//configure port mapping
////	MAP_PMAP_configurePorts((const uint8_t *) port_mapping,PMAP_P2MAP,1,PMAP_DISABLE_RECONFIGURATION);
//		pwm_red_Config_1.dutyCycle = 3750 * 255 / 255;
//		MAP_Timer_A_generatePWM(TIMER_A0_BASE, &pwm_red_Config_1);
//}