/*
 * ft6336u_driver.h
 *
 *  Created on: 8 dic. 2022
 *      Author: JoseArratia
 */

#ifndef MAIN_FT6336U_DRIVER_H_
#define MAIN_FT6336U_DRIVER_H_
/*INCLUDES*******************************************************************************************/
//#include "axp192_driver.h"
#include "i2c_bus_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*DEFINES********************************************************************************************/
#ifndef I2C_SYS_BUS
#define I2C_SYS_BUS
#define I2C_PORT		I2C_NUM_1
#define I2C_BAUD		400000
#define I2C_SDA			21
#define I2C_SCL			22
#endif

#define I2C_FT6_ADDR	0x38
#define EXT_FT6_PIN		39

#define FT6_DEVICE		"FT6 DRIVER"

#define FT6_QUEUE_SIZE	10

/*TYPEDESF******************************************************************************************/
typedef struct{
	uint16_t x;
	uint16_t y;
	//uint8_t state;
}tp_coord_t;

/*PROTOTYPES****************************************************************************************/
void ft6_init();
void ft6_get_touch(uint16_t *x, uint16_t *y, uint16_t *state);

QueueHandle_t ft6_get_queue();



#endif /* MAIN_FT6336U_DRIVER_H_ */
