
#ifndef _I2C_DRIVER_H_
#define _I2C_DRIVER_H_

//include "sensirion_arch_config.h"
#include <ti/devices/msp432p4xx/driverlib/i2c.h>
#include "driverlib.h"
#include "tick.h"
#include <stdio.h>

#define SGP30_SLAVE_ADDRESS 0x58

typedef enum {
	eUSCI_IDLE = 0,
	eUSCI_SUCCESS = 0,
	eUSCI_BUSY = 1,
	eUSCI_NACK = 2,
	eUSCI_STOP,
	eUSCI_START
} eUSCI_status;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

extern void sensirion_i2c_init(void);
extern int8_t sensirion_i2c_read(uint8_t address, uint8_t *data, uint16_t count);
extern int8_t sensirion_i2c_write(uint8_t address, const uint8_t *data,
                           uint8_t count);
void sensirion_sleep_usec(uint32_t useconds);

extern void bme280_initI2C(void);
extern int8_t bme280_writeI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui8ByteCount);
extern int8_t bme280_readI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui8ByteCount);
extern bool bme280_readBurstI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint16_t ui32ByteCount);

extern void initI2C(void);
extern bool writeI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount);
extern bool readI2CwithReg(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint8_t ui8ByteCount);
extern bool readBurstI2C(uint8_t ui8Addr, uint8_t ui8Reg, uint8_t *Data, uint32_t ui32ByteCount);



#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* SENSIRION_I2C_H */
