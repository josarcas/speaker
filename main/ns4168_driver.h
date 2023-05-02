/*
 * ns4168_driver.h
 *
 *  Created on: 10 dic. 2022
 *      Author: JoseArratia
 */

#ifndef MAIN_NS4168_DRIVER_H_
#define MAIN_NS4168_DRIVER_H_
/*INCLUDES***********************************************************************************************/
#include "ns4168_driver.h"

#include "driver/i2s_std.h"
#include "driver/gpio.h"

#include "axp192_driver.h"

/*DEFINES************************************************************************************************/
#ifndef I2S_SYS_BUS
#define I2S_SYS_BUS
#define I2S_PORT			I2S_NUM_0
#define I2S_SADTA_PIN		2
#define I2S_SIDTA_PIN		34				//Microphone
#define I2S_BCLK_PIN		12
#define I2S_LRCK_PIN		0
#endif

#define SAMPLE_RATE			44100

#define SPK_EN 				GPIO_2


/*PROTOTYPES*********************************************************************************************/
void ns4_init();

esp_err_t ns4_set_config(uint32_t s_rate, i2s_data_bit_width_t s_bits, i2s_slot_mode_t mode);

void ns4_enable(uint8_t ena);
void ns4_write_buffer(const uint8_t *buffer, uint32_t size);


#endif /* MAIN_NS4168_DRIVER_H_ */
