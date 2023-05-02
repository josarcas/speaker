/*
 * lv_esp_port.h
 *
 *  Created on: 20 dic. 2022
 *      Author: JoseArratia
 */

#ifndef MAIN_LV_ESP_PORT_H_
#define MAIN_LV_ESP_PORT_H_
/*INCLUDES****************************************************************************************/
#include "lvgl.h"
#include "axp192_driver.h"
#include "sd_spi_device.h"

/*DEFINES*****************************************************************************************/
//SPI BUS (LCD and SD)
#ifndef SPI_BUS
#define SPI_BUS
#define SYS_SPI_HOST				SPI2_HOST
#define SPI_MOSI_PIN				23
#define SPI_MISO_PIN				38
#define SPI_CLK_PIN					18
#endif

//LCD
#define LCD_CS_PIN				5
#define LCD_DC_PIN				15
#define LCD_RST_PIN				GPIO_4

#define LCD_CMD_BITS           	8
#define LCD_PARAM_BITS         	8
#define LCD_WIDTH				240
#define LCD_HEIGHT				320

#define LCD_PIXEL_CLOCK			(20 * 1000 * 1000)

#define LVGL_TICK_PERIOD_MS    	5

//AXP
#define LCD_BRIGTHNESS			DCDC_3

/*PROTOTYPES**************************************************************************************/
//LCD
void lv_esp_init();
void lv_esp_reset();

//TOUCH PAD
void lv_esp_touch_init();
void lv_esp_touch_set();

//FILE SYSTEM
void lv_esp_fs_init();
/*Image show*/
void lv_esp_img_show(const char *file_name);
lv_obj_t * espios_open_img(lv_obj_t *parent, char *file_name, uint8_t c0lor_fmt);


#endif /* MAIN_LV_ESP_PORT_H_ */
