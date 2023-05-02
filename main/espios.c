/*
 * espios.c
 *
 *  Created on: 4 ene. 2023
 *      Author: JoseArratia
 */
/*INCLUDES*************************************************************************************/
#include "espios.h"

/*DEFINES**************************************************************************************/
#define FS_CURRENT_SOUND				"ESPIOS/s.bin"

/*GLOBAL VARIABLES*****************************************************************************/
//SOUND
static float volume;

/*PROTOTYPES***********************************************************************************/
//SYSTEM
static void sys_spi_init_bus();

/*FUNCTIONS*************************************************************************************/
//SYSTEM
static void sys_spi_init_bus()
{
    spi_bus_config_t buscfg = {
        .sclk_io_num = SPI_CLK_PIN,
        .mosi_io_num = SPI_MOSI_PIN,
        .miso_io_num = SPI_MISO_PIN,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_HEIGHT * 80 * sizeof(uint16_t),
    };
    esp_err_t status = spi_bus_initialize(SYS_SPI_HOST, &buscfg, SPI_DMA_CH_AUTO);
}

void espios_init()
{
	//INIT NVS
	ESP_ERROR_CHECK(nvs_flash_init());

	//INIT I2S_BUS
	ns4_init();													//Allways init first
	espios_set_volume(50);

	//INIT I2C BUS
	i2c_bus_init();


	//INIT POWER MANAGER UNIT
	axp192_init();

	axp192_set_charge_current(CHARGE_Current_100mA);
	axp192_set_charge_volt(CHARGE_VOLT_4200mV);
	axp192_set_enable_charge(1);

	axp192_set_ldo_volt(PERI_VDD, 3300);
	axp192_set_ldo_enable(PERI_VDD, 1);
	axp192_set_ldo_enable(VIB_MOTOR, 0);
	axp192_set_ldo_volt(VIB_MOTOR, 2000);

	axp192_set_gpio_mode(LED_GREEN, GPIO_MODE_OPENDRAIN);
	axp192_set_gpio_mode(SPK_EN, GPIO_MODE_OPENDRAIN);
	axp192_set_gpio_level(SPK_EN, 0);
	axp192_set_ext(1);										//Enable 5V BUS
	axp192_set_gpio_level(LED_GREEN, 1);

	//INIT SPI BUS
	sys_spi_init_bus();

	//CONFIGURE LCD DEVICE
	lv_esp_init();

	//CONFIGURE TOUCH PANEL
	lv_esp_touch_init();
	lv_esp_touch_set();

}
//SOUND
void espios_set_volume(uint8_t percent)
{
	volume = percent>100? volume : (float)percent/(float)100;
}

//Screen
void espios_clear_screen()
{
	lv_obj_clean(lv_scr_act());
}
