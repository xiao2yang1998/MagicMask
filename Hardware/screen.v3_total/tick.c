/**
 ******************************************************************************
 * @file    serial.c
 * @author  William Xu
 * @version V1.0.0
 * @date    9-Apr-2018
 * @brief   Tick for delay and timeout
 ******************************************************************************
 *
 * Copyright (c) 2009-2018 MXCHIP Co.,Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ******************************************************************************
 */

#include <stdio.h>
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
#include "system_msp432p401r.h"

/******************************************************************************
 *                              Function Definitions
 ******************************************************************************/

static volatile uint32_t _ticker_count = 0;

void SysTick_Handler(void)
{
	_ticker_count++;
//    if (_ticker_count%1000==0) GPIO_toggleOutputOnPin(GPIO_PORT_P1,GPIO_PIN0);
}

void ms_ticker_init(void)
{
	SysTick_Config(SystemCoreClock / 1000);
//	SysTick_Config(CS_getMCLK() / 1000);
}

uint32_t ms_ticker_read(void)
{
	return _ticker_count;
}

void delay_ms(volatile uint32_t delay)
{
	uint32_t tickstart = 0;
	tickstart = ms_ticker_read();
	while((ms_ticker_read() - tickstart) < delay);
}
