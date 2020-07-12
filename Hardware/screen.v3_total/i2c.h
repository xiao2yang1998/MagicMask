#ifndef I2C_H
#define I2C_H
#include <ti/devices/msp432p4xx/driverlib/driverlib.h>
void* i2c_init(void *config);
int32_t i2c_cmd_write(uint16_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t length);
int32_t i2c_cmd_read(uint16_t slave_addr, uint8_t reg, uint8_t *buffer, uint8_t length);
#endif
