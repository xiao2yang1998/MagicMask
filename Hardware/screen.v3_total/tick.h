#ifndef _TICK_H
#define _TICK_H
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
void ms_ticker_init(void);
void delay_ms(volatile uint32_t delay);
#endif
