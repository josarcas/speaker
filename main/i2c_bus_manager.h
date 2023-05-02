/*
 * i2c_bus_manager.h
 *
 *  Created on: 10 dic. 2022
 *      Author: JoseArratia
 */

#ifndef MAIN_I2C_BUS_MANAGER_H_
#define MAIN_I2C_BUS_MANAGER_H_
/*INCLUDES******************************************************************************************/
#include "driver/i2c.h"
#include "stdint.h"

/*DEFINES*******************************************************************************************/
#ifndef I2C_SYS_BUS
#define I2C_SYS_BUS
#define I2C_PORT		I2C_NUM_1
#define I2C_BAUD		400000
#define I2C_SDA			21
#define I2C_SCL			22
#endif

/*PROTOTYPES****************************************************************************************/
void i2c_bus_init();

void i2c_bus_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t size);
void i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t size);

#endif /* MAIN_I2C_BUS_MANAGER_H_ */
