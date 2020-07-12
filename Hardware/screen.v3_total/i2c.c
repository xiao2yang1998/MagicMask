/**
 ******************************************************************************
 * @file    i2c.c
 * @author  William Xu
 * @version V1.0.0
 * @date    9-Apr-2018
 * @brief   I2C sensor rw driver
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
#include "tick.h"
#include <ti/devices/msp432p4xx/driverlib/i2c.h>

/******************************************************************************
 *                                 Structures
 ******************************************************************************/

struct i2c_instance {
    uint32_t place_holder;
};

/******************************************************************************
 *                              Variable Definitions
 ******************************************************************************/

/* I2C Master Configuration Parameter */
const eUSCI_I2C_MasterConfig i2cConfig_copy =
{
    EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    12000000,                               // SMCLK = 12MHz
//    EUSCI_B_I2C_SET_DATA_RATE_1MBPS,        // Desired I2C Clock of 1Mhz
    EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 100khz
    0,                                      // No byte counter threshold
    EUSCI_B_I2C_NO_AUTO_STOP                // No Autostop
};

#define I2C_TIMEOUT  5000

/* NULL handler for this driver, complete in future */
static struct i2c_instance i2c;

/******************************************************************************
 *                              Function Definitions
 ******************************************************************************/
 
void* i2c_init(void *config)
{
    /* Select Port 6 for I2C - Set Pin 4, 5 to input Primary Module Function,
     *   (UCB1SIMO/UCB1SDA, UCB1SOMI/UCB1SCL).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P6,
            GPIO_PIN4 + GPIO_PIN5, GPIO_PRIMARY_MODULE_FUNCTION);
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, 0x3c);
    /* Initializing I2C Master to SMCLK at 400kbs with no autostop */
    MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig_copy);

    /* Set Master in transmit mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Enable I2C Module to start operations */
    MAP_I2C_enableModule(EUSCI_B1_BASE);

#if 0
    /* Enable and clear the interrupt flag */
    MAP_I2C_clearInterruptFlag(EUSCI_B1_BASE,
            EUSCI_B_I2C_TRANSMIT_INTERRUPT0 + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    //Enable master Receive interrupt
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
    MAP_Interrupt_enableSleepOnIsrExit();
    MAP_Interrupt_enableInterrupt(INT_EUSCIB0);
#endif

    return &i2c;

}


/**
 * \brief Sync version of i2c write command
 */
int32_t i2c_cmd_write(uint16_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t length)
{
    int err = 0;
    
    //EUSCI_B_I2C_SENDING_STOP:Transmit STOP condition in master mode
    while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);

    /* Set Master in transmit mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

    /* Specify slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, slave_addr);
    //Transmits the first data byte of a multi-byte transmission to the Slave
    //主机向从机发送多字节数据的第一个命令字节
    MAP_I2C_masterSendMultiByteStartWithTimeout(EUSCI_B1_BASE, reg, I2C_TIMEOUT);

//    if (length) MAP_I2C_masterSendMultiByteNextWithTimeout(EUSCI_B1_BASE, reg, I2C_TIMEOUT);

    while (length--)
    {
        MAP_I2C_masterSendMultiByteNextWithTimeout(EUSCI_B1_BASE, *buffer++, I2C_TIMEOUT);
    }

    MAP_I2C_masterSendMultiByteStopWithTimeout(EUSCI_B1_BASE, I2C_TIMEOUT);
	return err;
}

int32_t i2c_cmd_read(uint16_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t length)
{
    uint8_t i;
    /* Specify slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, slave_addr);

    /* Set Master in transmit mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_MODE);
    //Indicates whether STOP got sent.
    while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);
    
    //Starts multi-byte transmission from Master to Slave
    I2C_masterSendMultiByteStart(EUSCI_B1_BASE, reg);
    //Send STOP byte at the end of a multi-byte transmission from Master to Slave
    I2C_masterSendMultiByteStop(EUSCI_B1_BASE);
   
    //Indicates whether STOP got sent.
    while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);
    /* Set Master in receive mode */
    MAP_I2C_setMode(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_MODE);
    delay_ms(100);

    for(;;)
    {
        while (MAP_I2C_masterIsStopSent(EUSCI_B1_BASE) == EUSCI_B_I2C_SENDING_STOP);
        I2C_masterReceiveStart(EUSCI_B1_BASE);
        //Returns true if the START has been sent, false if it is sending
        while(I2C_masterIsStartSent(EUSCI_B1_BASE));
        //if Not-acknowledge interrupt
        if(I2C_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_I2C_NAK_INTERRUPT))
        {
            I2C_clearInterruptFlag(EUSCI_B1_BASE, EUSCI_B_I2C_NAK_INTERRUPT);
            //Sends the STOP at the end of a multi-byte reception at the Master end
            I2C_masterReceiveMultiByteStop(EUSCI_B1_BASE);
            //Indicates whether STOP got sent.
            while(I2C_masterIsStopSent(EUSCI_B1_BASE));
            delay_ms(10);
        }
        else
        {
            for(i=0; i<length-1; i++)
            {
                //Does single byte reception from the slave
                buffer[i] = I2C_masterReceiveSingle(EUSCI_B1_BASE);
            }
            
            buffer[i] = I2C_masterReceiveMultiByteFinish(EUSCI_B1_BASE);
            break;
        }
    }

    return 0;
}
