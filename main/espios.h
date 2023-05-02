/*
 * espios.h
 *
 *  Created on: 4 ene. 2023
 *      Author: JoseArratia
 */

#ifndef MAIN_ESPIOS_H_
#define MAIN_ESPIOS_H_
/*INCLUDES*************************************************************************************/
#include "stdio.h"
#include "stdbool.h"
#include "string.h"
#include "stdlib.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "nvs.h"

#include "i2c_bus_manager.h"
#include "axp192_driver.h"
#include "sd_spi_device.h"
#include "ns4168_driver.h"
#include "lv_esp_port.h"

#include "espios_bt.h"

/*DEFINES***************************************************************************************/
//SPI BUS (LCD and SD)
#ifndef SPI_BUS
#define SPI_BUS
#define SYS_SPI_HOST				SPI2_HOST
#define SPI_MOSI_PIN				23
#define SPI_MISO_PIN				38
#define SPI_CLK_PIN					18
#endif

//SYSTEM
/*AXP port*/
#define LED_GREEN				GPIO_1
//#ifndef SPK_EN
//#define SPK_EN 					GPIO_2
//#endif
#define VIB_MOTOR				LDO_3
#define PERI_VDD				LDO_2			//LCD, SD module VDD
#define LCD_BRIGTHNESS			DCDC_3

//NOFICATIONS PRIOTITY
#define VERY_LOW_PRIORITY		0
#define LOW_PRIORITY			1
#define MEDIUM_PRIORITY			2
#define HIGH_PRIORITY			3

#define NOTIFI_QUEUE_SIZE		10

#define MAX_STRING_SIZE			100

//SD TARGET
#define INIT_FILE_CONFIG_NAME "/sd/ESPIOS/config.txt"

//WIFI SERVICES
#define E_NONE		 			0x00
#define	E_SNTP		 			0x01							//Time protocol
#define	E_SMTP		 			0x02							//Out mail
#define	E_POP					0x04							//In mail
#define	E_WEATHER	 			0x08
#define	E_ALL		 			E_SNTP|E_SMTP|E_POP|E_WEATHER

//LED BAR ANIMATION(SK6812)
/*Animations*/
#define SIMPLE_COLOR			0
#define LINEAR_DEG				1
#define C_BRIGHTNESS			2
#define C_FALL					3
/*Extra data*/
#define L_BAR_DELAY				1
#define L_BAR_LVL				100

/*TYPEDEFS**************************************************************************************/

/*PROTOTYPES************************************************************************************/
void espios_init();

//SOUND
void espios_set_volume(uint8_t percent);

//Screen
void espios_clear_screen();

#endif /* MAIN_ESPIOS_H_ */
