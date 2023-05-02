/*
 * i2c_bus_manager.c
 *
 *  Created on: 10 dic. 2022
 *      Author: JoseArratia
 */
/*INCLUDES******************************************************************************************/
#include "i2c_bus_manager.h"

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"

/*GLOBLA VARIOABLES*********************************************************************************/
static SemaphoreHandle_t i2c_mutex = NULL;

/*FUNCTIONS*****************************************************************************************/
void i2c_bus_init()
{
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_BAUD,
        // .clk_flags = 0,          /*!< Optional, you can use I2C_SCLK_SRC_FLAG_* flags to choose i2c source clock here. */
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));

    i2c_mutex = xSemaphoreCreateMutex();
}

void i2c_bus_write(uint8_t addr, uint8_t reg, uint8_t *data, uint16_t size)
{
	if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
	{
	    i2c_cmd_handle_t write_cmd = i2c_cmd_link_create();
	    i2c_master_start(write_cmd);
	    i2c_master_write_byte(write_cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
	    i2c_master_write_byte(write_cmd, reg, 1);

	    if (size > 0) {
	        i2c_master_write(write_cmd, data, size, 1);
	    }
	    i2c_master_stop(write_cmd);

	    //esp_err_t err = ESP_FAIL;

	    /*err =*/ i2c_master_cmd_begin(I2C_PORT, write_cmd, 1000000);

	    i2c_cmd_link_delete(write_cmd);


	    //ESP_ERROR_CHECK(err);
	    xSemaphoreGive(i2c_mutex);
	}
}

void i2c_bus_read(uint8_t addr, uint8_t reg, uint8_t *data, uint8_t size)
{
	if(xSemaphoreTake(i2c_mutex, portMAX_DELAY) == pdTRUE)
	{
	    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

		i2c_master_start(cmd);
		i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_WRITE, 1);
		i2c_master_write_byte(cmd, reg, 1);

	    i2c_master_start(cmd);
	    i2c_master_write_byte(cmd, (addr << 1) | I2C_MASTER_READ, 1);
	    if (size > 1) {
	        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
	    }
	    if (size > 0) {
	        i2c_master_read_byte(cmd, &data[size-1], I2C_MASTER_NACK);
	    }
	    i2c_master_stop(cmd);

	    //esp_err_t err = ESP_FAIL;

	    /*err = */i2c_master_cmd_begin(I2C_PORT, cmd, 1000000);
	    i2c_cmd_link_delete(cmd);

	    //ESP_ERROR_CHECK(err);
	    xSemaphoreGive(i2c_mutex);
	}
}



