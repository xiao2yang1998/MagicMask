#include <stdbool.h>
#include <stdint.h>
#include "msp432.h"
#include "i2c_driver.h"
#include "driverlib.h"
/*
 * INSTRUCTIONS
 * ============
 *
 * Implement all functions where they are marked as IMPLEMENT.
 * Follow the function specification in the comments.
 */

//*****************************************************************************
//
// Global Data
//
//*****************************************************************************
volatile eUSCI_status ui8Status;

uint8_t  *pData;
uint8_t  ui8DummyRead;
uint32_t g_ui32ByteCount;
bool     burstMode = false;
//void euscib1IntHandler(void);

/* I2C Master Configuration Parameter */
volatile eUSCI_I2C_MasterConfig i2cConfig =
{
    EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    0,
    EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
    0,                                      // No byte counter threshold
    EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD                // Autostop
};
//SGP30
volatile eUSCI_I2C_MasterConfig i2cConfig_B0 =
{
    EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    12000000,                               // SMCLK = 12MHz
//    EUSCI_B_I2C_SET_DATA_RATE_1MBPS,        // Desired I2C Clock of 1Mhz
    EUSCI_B_I2C_SET_DATA_RATE_100KBPS,      // Desired I2C Clock of 100khz
    0,                                      // No byte counter threshold
    EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD                // No Autostop
};

//MAX30100
volatile eUSCI_I2C_MasterConfig i2cConfig_B1 =
{
        EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
		0,
		EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
        0,                                      // No byte counter threshold
        EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD                // Autostop
};

//BME280
volatile eUSCI_I2C_MasterConfig i2cConfig_B2 =
{
    EUSCI_B_I2C_CLOCKSOURCE_SMCLK,          // SMCLK Clock Source
    0,
    EUSCI_B_I2C_SET_DATA_RATE_400KBPS,      // Desired I2C Clock of 400khz
    0,                                      // No byte counter threshold
    EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD                // Autostop
};

/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
/**
 * Initialize all hard- and software components that are needed for the I2C
 * communication.
 */
void sensirion_i2c_init() {
    /* Select Port 6 for I2C - Set Pin 4, 5 to input Primary Module Function,
     *   (UCB1SIMO/UCB1SDA, UCB1SOMI/UCB1SCL).
     */
    MAP_GPIO_setAsPeripheralModuleFunctionInputPin(GPIO_PORT_P1,
            GPIO_PIN7 + GPIO_PIN6, GPIO_PRIMARY_MODULE_FUNCTION);
//    MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, SGP30_SLAVE_ADDRESS);   
//	//     /* Initializing I2C Master to SMCLK at 400kbs with no autostop */
    MAP_I2C_initMaster(EUSCI_B0_BASE, &i2cConfig_B0);

//    // /* Set Master in transmit mode */
//    MAP_I2C_setMode(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_MODE);

//    // /* Enable I2C Module to start operations */
//    MAP_I2C_enableModule(EUSCI_B0_BASE);
}

/**
 * Execute one read transaction on the I2C bus, reading a given number of bytes.
 * If the device does not acknowledge the read command, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to read from
 * @param data    pointer to the buffer where the data is to be stored
 * @param count   number of bytes to read from I2C and store in the buffer
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_read(uint8_t address, uint8_t *data, uint16_t count) {
    /* Todo: Put a delay */
	/* Wait until ready */
    while (MAP_I2C_isBusBusy(EUSCI_B0_BASE));

	/* Assign Data to local Pointer */
		pData = data;
		g_ui32ByteCount = count;
    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B0_BASE);

  	/* Setup the number of bytes to receive */
    i2cConfig_B0.byteCounterThreshold = count;
    i2cConfig_B0.autoSTOPGeneration = EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD;
    MAP_I2C_initMaster(EUSCI_B0_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig_B0);

	/* Load device slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, address);

    /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B0_BASE);

  	/* Enable master STOP and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
    		EUSCI_B_I2C_NAK_INTERRUPT);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

  	/* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB0);


  	/* Turn off TX and generate RE-Start */
  	MAP_I2C_masterReceiveStart(EUSCI_B0_BASE);

  	/* Enable RX interrupt */
    MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

	/* Wait for all data be received */
	while(ui8Status == eUSCI_BUSY)
	{

#ifdef USE_LPM
		MAP_PCM_gotoLPM0();
#else
		__no_operation();
#endif
	}

	/* Disable interrupts */
	MAP_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB0);

	if(ui8Status == eUSCI_NACK)
	{
		return(1);
	}
	else
	{
		return(0);
	}

}

/**
 * Execute one write transaction on the I2C bus, sending a given number of
 * bytes. The bytes in the supplied buffer must be sent to the given address. If
 * the slave device does not acknowledge any of the bytes, an error shall be
 * returned.
 *
 * @param address 7-bit I2C address to write to
 * @param data    pointer to the buffer containing the data to write
 * @param count   number of bytes to read from the buffer and send over I2C
 * @returns 0 on success, error code otherwise
 */
int8_t sensirion_i2c_write(const uint8_t address, const uint8_t *data,
                           uint8_t count) {
		//printf("start write\n");
	/* Wait until ready to write */
		//printf("write\n");
    while(MAP_I2C_isBusBusy(EUSCI_B0_BASE));
	/* Assign Data to local Pointer */
	uint8_t ui8Reg = data[0];
	data++;
	pData = data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B0_BASE);	
	
	/* Setup the number of bytes to transmit + 1 to account for the register byte */
    i2cConfig_B0.byteCounterThreshold =(uint8_t)count ;
    MAP_I2C_initMaster(EUSCI_B0_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig_B0);
	/* Load device slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B0_BASE, address);
    /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B0_BASE);
  	/* Enable master STOP, TX and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
    		EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;
	/* Send start bit and register */
  	MAP_I2C_masterSendMultiByteStart(EUSCI_B0_BASE,ui8Reg);
  	/* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB0);

	// NOW WAIT FOR DATA BYTES TO BE SENT
	while(ui8Status == eUSCI_BUSY)
	{
#ifdef USE_LPM
		MAP_PCM_gotoLPM0();
#else
		__no_operation();
#endif
	}

	/* Disable interrupts */
	MAP_I2C_disableInterrupt(EUSCI_B0_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB0);
	if(ui8Status == eUSCI_NACK)
	{//FALSE
		return(1);
	}
	else
	{
		//SUCCESS
		return(0);
	}
}

/**
 * Sleep for a given number of microseconds. The function should delay the
 * execution for at least the given time, but may also sleep longer.
 *
 * Despite the unit, a <10 millisecond precision is sufficient.
 *
 * @param useconds the sleep time in microseconds
 */


void sensirion_sleep_usec(uint32_t useconds) {
	  delay_ms(useconds/1000 + 1);
    return;
}

void bme280_initI2C(void)
{
    /* I2C Clock Soruce Speed */
    i2cConfig.i2cClk = MAP_CS_getSMCLK();

    /* Select I2C function for I2C_SCL(P6.5) & I2C_SDA(P6.4) */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5 | GPIO_PIN4,
            GPIO_PRIMARY_MODULE_FUNCTION);

    /* Initializing I2C Master to SMCLK at 400kbs with autostop */
//    MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig);
}

/***********************************************************
  Function:
*/
int8_t bme280_writeI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui8ByteCount)
{
    // printf("---------start write\n");
    // printf("---------address:%x reg:%x count:%d\n",ui8Addr,ui8Reg,ui8ByteCount);
    // for(int i=0;i<ui8ByteCount;i++){
    // 	printf("---------%d  %x\n",i+1,Data[i]);
    // }
    /* Wait until ready to write */
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));

    /* Assign Data to local Pointer */
    pData = Data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

    /* Setup the number of bytes to transmit + 1 to account for the register byte */
    i2cConfig.byteCounterThreshold = ui8ByteCount + 1;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig);

    /* Load device slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
    MAP_I2C_enableModule(EUSCI_B1_BASE);

    /* Enable master STOP, TX and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                            EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

    /* Send start bit and register */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

    /* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

    // NOW WAIT FOR DATA BYTES TO BE SENT
    while(ui8Status == eUSCI_BUSY)
    {
#ifdef USE_LPM
        MAP_PCM_gotoLPM0();
#else
        __no_operation();
#endif
    }

    /* Disable interrupts */
    MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                             EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);

    if(ui8Status == eUSCI_NACK)
    {
        // printf("---------finish wirte unsuccess\n");
        return(-1);
    }
    else
    {
        // printf("---------finish wirte success\n\n");
        return(0);
    }
}

/***********************************************************
  Function:
*/
int8_t bme280_readI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui8ByteCount)
{
    /* Todo: Put a delay */
    /* Wait until ready */
//	printf("---------start read\n");
//	printf("---------address:%x reg:%x count:%d\n",ui8Addr,ui8Reg,ui8ByteCount);
    //return 13;
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));

    /* Assign Data to local Pointer */
    pData = Data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

    /* Setup the number of bytes to receive */
    i2cConfig.byteCounterThreshold = ui8ByteCount;
    i2cConfig.autoSTOPGeneration = EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig);

    /* Load device slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
    MAP_I2C_enableModule(EUSCI_B1_BASE);

    /* Enable master STOP and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                            EUSCI_B_I2C_NAK_INTERRUPT);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

    /* Send start bit and register */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

    /* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

    /* NOTE: If the number of bytes to receive = 1, then as target register is being shifted
     * out during the write phase, UCBxTBCNT will be counted and will trigger STOP bit prematurely
     * If count is > 1, wait for the next TXBUF empty interrupt (just after reg value has been
     * shifted out
     */
    while(ui8Status == eUSCI_BUSY)
    {
        if(MAP_I2C_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0))
        {
            ui8Status = eUSCI_IDLE;
        }
    }

    ui8Status = eUSCI_BUSY;

    /* Turn off TX and generate RE-Start */
    MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);

    /* Enable RX interrupt */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

    /* Wait for all data be received */
    while(ui8Status == eUSCI_BUSY)
    {

#ifdef USE_LPM
        MAP_PCM_gotoLPM0();
#else
        __no_operation();
#endif
    }

    /* Disable interrupts */
    MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                             EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);




    if(ui8Status == eUSCI_NACK)
    {
        //	printf("---------finish read unsuccess\n");
        return(-1);
    }
    else
    {
//		printf("---------finish read success\n");
//		for(int i=0;i<ui8ByteCount;i++)
//			printf("--------- %d  %x\n",i+1,Data[i]);
//		printf("\n");
        return(0);
    }
}

/***********************************************************
  Function:
*/
bool bme280_readBurstI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui32ByteCount)
{
    /* Todo: Put a delay */
    /* Wait until ready */
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));

    /* Assign Data to local Pointer */
    pData = Data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

    /* Setup the number of bytes to receive */
    i2cConfig.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
    g_ui32ByteCount = ui32ByteCount;
    burstMode = true;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig);

    /* Load device slave address */
    MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
    MAP_I2C_enableModule(EUSCI_B1_BASE);

    /* Enable master STOP and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                            EUSCI_B_I2C_NAK_INTERRUPT);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

    /* Send start bit and register */
    MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

    /* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

    /* NOTE: If the number of bytes to receive = 1, then as target register is being shifted
     * out during the write phase, UCBxTBCNT will be counted and will trigger STOP bit prematurely
     * If count is > 1, wait for the next TXBUF empty interrupt (just after reg value has been
     * shifted out
     */
    while(ui8Status == eUSCI_BUSY)
    {
        if(MAP_I2C_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0))
        {
            ui8Status = eUSCI_IDLE;
        }
    }

    ui8Status = eUSCI_BUSY;

    /* Turn off TX and generate RE-Start */
    MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);

    /* Enable RX interrupt */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

    /* Wait for all data be received */
    while(ui8Status == eUSCI_BUSY)
    {

#ifdef USE_LPM
        MAP_PCM_gotoLPM0();
#else
        __no_operation();
#endif
    }

    /* Disable interrupts */
    MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
                             EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);

    if(ui8Status == eUSCI_NACK)
    {
        return(-1);
    }
    else
    {
        return(0);
    }
}

/***********************************************************
  Function:
*/
void initI2C(void)
{
	/* I2C Clock Soruce Speed */
	i2cConfig_B1.i2cClk = MAP_CS_getSMCLK();

    /* Select I2C function for I2C_SCL(P6.5) & I2C_SDA(P6.4) */
    GPIO_setAsPeripheralModuleFunctionOutputPin(GPIO_PORT_P6, GPIO_PIN5 | GPIO_PIN4,
            GPIO_PRIMARY_MODULE_FUNCTION);

    /* Initializing I2C Master to SMCLK at 400kbs with autostop */
    MAP_I2C_initMaster(EUSCI_B1_BASE, &i2cConfig_B1);
}

/***********************************************************
  Function:
*/
bool writeI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount)
{	
	// printf("---------start write\n");
	// printf("---------address:%x reg:%x count:%d\n",ui8Addr,ui8Reg,ui8ByteCount);
	// for(int i=0;i<ui8ByteCount;i++){
	// 	printf("---------%d  %x\n",i+1,Data[i]);
	// }
	/* Wait until ready to write */
		//printf("reg %x\n",ui8Reg);
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
	// printf("busy end\n");
	/* Assign Data to local Pointer */
	pData = Data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

	/* Setup the number of bytes to transmit + 1 to account for the register byte */
    i2cConfig_B1.byteCounterThreshold = ui8ByteCount + 1;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig_B1);

	/* Load device slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B1_BASE);

  	/* Enable master STOP, TX and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
    		EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

	/* Send start bit and register */
  	MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

  	/* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

	// NOW WAIT FOR DATA BYTES TO BE SENT
	while(ui8Status == eUSCI_BUSY)
	{
#ifdef USE_LPM
		MAP_PCM_gotoLPM0();
#else
		__no_operation();
#endif
	}

	/* Disable interrupts */
	MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_TRANSMIT_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);

	if(ui8Status == eUSCI_NACK)
	{	
		// printf("---------finish wirte unsuccess\n");
		return(false);
	}
	else
	{	
		// printf("---------finish wirte success\n\n");
		return(true);
	}
}

/***********************************************************
  Function:
*/
bool readI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount)
{
	/* Todo: Put a delay */
	/* Wait until ready */
	// printf("---------start read\n");
//	printf("---------address:%x reg:%x count:%d\n",ui8Addr,ui8Reg,ui8ByteCount);
		//printf("reg %x\n",ui8Reg);
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));
		// printf("busy end\n");
		//printf("Reg %x\n",ui8Reg);
	/* Assign Data to local Pointer */
		pData = Data;
		g_ui32ByteCount = ui8ByteCount;
    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

  	/* Setup the number of bytes to receive */
    i2cConfig_B1.byteCounterThreshold = ui8ByteCount;
    i2cConfig_B1.autoSTOPGeneration = EUSCI_B_I2C_SEND_STOP_AUTOMATICALLY_ON_BYTECOUNT_THRESHOLD;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig_B1);

	/* Load device slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B1_BASE);

  	/* Enable master STOP and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
    		EUSCI_B_I2C_NAK_INTERRUPT);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

  	/* Send start bit and register */
  	MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

  	/* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

  	/* NOTE: If the number of bytes to receive = 1, then as target register is being shifted
  	 * out during the write phase, UCBxTBCNT will be counted and will trigger STOP bit prematurely
  	 * If count is > 1, wait for the next TXBUF empty interrupt (just after reg value has been
  	 * shifted out
  	 */
	while(ui8Status == eUSCI_BUSY)
	{
		if(MAP_I2C_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0))
		{
			ui8Status = eUSCI_IDLE;
		}
	}

	ui8Status = eUSCI_BUSY;
  	/* Turn off TX and generate RE-Start */
  	MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);

  	/* Enable RX interrupt */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

	/* Wait for all data be received */
	while(ui8Status == eUSCI_BUSY)
	{

#ifdef USE_LPM
		MAP_PCM_gotoLPM0();
#else
		__no_operation();
#endif
	}

	/* Disable interrupts */
	MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);


	if(ui8Status == eUSCI_NACK)
	{	
	//	printf("---------finish read unsuccess\n");		
		return(false);
	}
	else
	{
		//printf("---------finish read success\n");
//		for(int i=0;i<ui8ByteCount;i++)
//			printf("--------- %d  %x\n",i+1,Data[i]);
//		printf("\n");
		return(true);
	}
}

bool readBurstI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint32_t ui32ByteCount)
{
	/* Todo: Put a delay */
	/* Wait until ready */
    while (MAP_I2C_isBusBusy(EUSCI_B1_BASE));

	/* Assign Data to local Pointer */
	pData = Data;

    /* Disable I2C module to make changes */
    MAP_I2C_disableModule(EUSCI_B1_BASE);

  	/* Setup the number of bytes to receive */
    i2cConfig_B1.autoSTOPGeneration = EUSCI_B_I2C_NO_AUTO_STOP;
    g_ui32ByteCount = ui32ByteCount;
    burstMode = true;
    MAP_I2C_initMaster(EUSCI_B1_BASE, (const eUSCI_I2C_MasterConfig *)&i2cConfig_B1);

	/* Load device slave address */
	MAP_I2C_setSlaveAddress(EUSCI_B1_BASE, ui8Addr);

    /* Enable I2C Module to start operations */
	MAP_I2C_enableModule(EUSCI_B1_BASE);

  	/* Enable master STOP and NACK interrupts */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
    		EUSCI_B_I2C_NAK_INTERRUPT);

    /* Set our local state to Busy */
    ui8Status = eUSCI_BUSY;

  	/* Send start bit and register */
  	MAP_I2C_masterSendMultiByteStart(EUSCI_B1_BASE,ui8Reg);

  	/* Enable master interrupt for the remaining data */
    MAP_Interrupt_enableInterrupt(INT_EUSCIB1);

  	/* NOTE: If the number of bytes to receive = 1, then as target register is being shifted
  	 * out during the write phase, UCBxTBCNT will be counted and will trigger STOP bit prematurely
  	 * If count is > 1, wait for the next TXBUF empty interrupt (just after reg value has been
  	 * shifted out
  	 */
	while(ui8Status == eUSCI_BUSY)
	{
		if(MAP_I2C_getInterruptStatus(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0))
		{
			ui8Status = eUSCI_IDLE;
		}
	}

	ui8Status = eUSCI_BUSY;

  	/* Turn off TX and generate RE-Start */
  	MAP_I2C_masterReceiveStart(EUSCI_B1_BASE);

  	/* Enable RX interrupt */
    MAP_I2C_enableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_RECEIVE_INTERRUPT0);

	/* Wait for all data be received */
	while(ui8Status == eUSCI_BUSY)
	{

#ifdef USE_LPM
		MAP_PCM_gotoLPM0();
#else
		__no_operation();
#endif
	}

	/* Disable interrupts */
	MAP_I2C_disableInterrupt(EUSCI_B1_BASE, EUSCI_B_I2C_STOP_INTERRUPT +
			EUSCI_B_I2C_NAK_INTERRUPT + EUSCI_B_I2C_RECEIVE_INTERRUPT0);
    MAP_Interrupt_disableInterrupt(INT_EUSCIB1);

	if(ui8Status == eUSCI_NACK)
	{
		return(false);
	}
	else
	{
		return(true);
	}
}


/***********************************************************
  Function: euscib1IntHandler
 */
void EUSCIB0_IRQHandler(void)
	{		
		//printf("EUSCIB0_IRQHandler :\n");
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(EUSCI_B0_BASE);
    MAP_I2C_clearInterruptFlag(EUSCI_B0_BASE, status);

    if (status & EUSCI_B_I2C_NAK_INTERRUPT)
    {	
			//printf("EUSCI_B_I2C_NAK_INTERRUPT\n");
    	/* Generate STOP when slave NACKS */
        MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE);

    	/* Clear any pending TX interrupts */
    	MAP_I2C_clearInterruptFlag(EUSCI_B0_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

        /* Set our local state to NACK received */
        ui8Status = eUSCI_NACK;
    }

    if (status & EUSCI_B_I2C_START_INTERRUPT)
    {	
			//printf("EUSCI_B_I2C_START_INTERRUPT\n");
        /* Change our local state */
        ui8Status = eUSCI_START;
    }

    if (status & EUSCI_B_I2C_STOP_INTERRUPT)
    {
			//printf("iterrupt\n");
			//printf("EUSCI_B_I2C_STOP_INTERRUPT\n");
        /* Change our local state */
        ui8Status = eUSCI_STOP;
    }

    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
    {
			//printf("EUSCI_B_I2C_RECEIVE_INTERRUPT0\n");
    	/* RX data */
			
    	*pData++ = MAP_I2C_masterReceiveMultiByteNext(EUSCI_B0_BASE);
    	ui8DummyRead= MAP_I2C_masterReceiveMultiByteNext(EUSCI_B0_BASE);
			//printf("  1 %x   2 %x  count %x\n",*(pData-1),ui8DummyRead,g_ui32ByteCount);
    	if (burstMode)
    	{
    		g_ui32ByteCount--;
    		if (g_ui32ByteCount == 1)
    		{
    			burstMode = false;

    			/* Generate STOP */
    	        MAP_I2C_masterSendMultiByteStop(EUSCI_B0_BASE);
    		}
    	}
    }

    if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0)
    {
			//printf("EUSCI_B_I2C_TRANSMIT_INTERRUPT0\n");
    	/* Send the next data */
			//printf("%x\n",*pData);
    	MAP_I2C_masterSendMultiByteNext(EUSCI_B0_BASE, *pData++);
    }

#ifdef USE_LPM
    MAP_Interrupt_disableSleepOnIsrExit();
#endif
}

void EUSCIB1_IRQHandler(void)
{
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(EUSCI_B1_BASE);
    MAP_I2C_clearInterruptFlag(EUSCI_B1_BASE, status);
		printf("euscibi interrupt\n");
    if (status & EUSCI_B_I2C_NAK_INTERRUPT)
    {
    	/* Generate STOP when slave NACKS */
        MAP_I2C_masterSendMultiByteStop(EUSCI_B1_BASE);

    	/* Clear any pending TX interrupts */
    	MAP_I2C_clearInterruptFlag(EUSCI_B1_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

        /* Set our local state to NACK received */
        ui8Status = eUSCI_NACK;
    }

    if (status & EUSCI_B_I2C_START_INTERRUPT)
    {
			printf("start interrupt\n");
        /* Change our local state */
        ui8Status = eUSCI_START;
    }

    if (status & EUSCI_B_I2C_STOP_INTERRUPT)
    {		
			//printf("stop interrupt\n");
        /* Change our local state */
        ui8Status = eUSCI_STOP;
    }

    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
    {
    	/* RX data */
    	*pData++ = MAP_I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
    	ui8DummyRead= MAP_I2C_masterReceiveMultiByteNext(EUSCI_B1_BASE);
			//printf("  1 %x   2%x  count %x\n",*(pData-1),ui8DummyRead,g_ui32ByteCount);
    	if (burstMode)
    	{
    		g_ui32ByteCount--;
    		if (g_ui32ByteCount == 1)
    		{
    			burstMode = false;

    			/* Generate STOP */
    	        MAP_I2C_masterSendMultiByteStop(EUSCI_B1_BASE);
    		}
    	}
    }

    if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0)
    {
    	/* Send the next data */
    	MAP_I2C_masterSendMultiByteNext(EUSCI_B1_BASE, *pData++);
    }

#ifdef USE_LPM
    MAP_Interrupt_disableSleepOnIsrExit();
#endif
}

void EUSCIB2_IRQHandler(void)
{
    uint_fast16_t status;

    status = MAP_I2C_getEnabledInterruptStatus(EUSCI_B2_BASE);
    MAP_I2C_clearInterruptFlag(EUSCI_B2_BASE, status);

    if (status & EUSCI_B_I2C_NAK_INTERRUPT)
    {
        /* Generate STOP when slave NACKS */
        MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);

        /* Clear any pending TX interrupts */
        MAP_I2C_clearInterruptFlag(EUSCI_B2_BASE, EUSCI_B_I2C_TRANSMIT_INTERRUPT0);

        /* Set our local state to NACK received */
        ui8Status = eUSCI_NACK;
    }

    if (status & EUSCI_B_I2C_START_INTERRUPT)
    {
        /* Change our local state */
        ui8Status = eUSCI_START;
    }

    if (status & EUSCI_B_I2C_STOP_INTERRUPT)
    {
        /* Change our local state */
        ui8Status = eUSCI_STOP;
    }

    if (status & EUSCI_B_I2C_RECEIVE_INTERRUPT0)
    {
        /* RX data */
        *pData++ = MAP_I2C_masterReceiveMultiByteNext(EUSCI_B2_BASE);
        ui8DummyRead= MAP_I2C_masterReceiveMultiByteNext(EUSCI_B2_BASE);

        if (burstMode)
        {
            g_ui32ByteCount--;
            if (g_ui32ByteCount == 1)
            {
                burstMode = 0;

                /* Generate STOP */
                MAP_I2C_masterSendMultiByteStop(EUSCI_B2_BASE);
            }
        }
    }

    if (status & EUSCI_B_I2C_TRANSMIT_INTERRUPT0)
    {
        /* Send the next data */
        MAP_I2C_masterSendMultiByteNext(EUSCI_B2_BASE, *pData++);
    }

#ifdef USE_LPM
    MAP_Interrupt_disableSleepOnIsrExit();
#endif
}



