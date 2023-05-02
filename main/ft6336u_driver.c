/*
 * ft6336u_driver.c
 *
 *  Created on: 8 dic. 2022
 *      Author: JoseArratia
 */
/*INCLUDES************************************************************************************/
#include "driver/i2c.h"
#include "driver/gpio.h"
#include "esp_log.h"

#include "ft6336u_driver.h"

/*GLOBAL VARIALBES****************************************************************************/
//static uint16_t x_pos;
//static uint16_t y_pos;
//static uint8_t t_state;

static tp_coord_t tp_data;

static SemaphoreHandle_t ft6_sem = NULL;
static QueueHandle_t ft6_data_queue = NULL;

/*PROTOTYPES*********************************************************************************/
//static void IRAM_ATTR ft6_isr_handler(void* arg);

static void ft6_task(void *args);

//static void ft6_read_reg(uint8_t reg, uint8_t *data, uint16_t size);
//static void ft6_write_reg(uint8_t reg, uint8_t *data, uint16_t size);


/*FUNCTIONS**********************************************************************************/
static void IRAM_ATTR ft6_isr_handler(void* arg)
{
	xSemaphoreGiveFromISR(ft6_sem, NULL);
}

static void ft6_task(void *args)
{
	uint8_t buffer[4];

	//ESP_LOGI(FT6_DEVICE, "Touch task init");
	for(;;)
	{
		if(xSemaphoreTake(ft6_sem, portMAX_DELAY) == pdTRUE)
		{
			//ESP_LOGI(FT6_DEVICE, "Pressed on %d, %d", x_pos, y_pos);
			while(!gpio_get_level(EXT_FT6_PIN))
			{
				i2c_bus_read(I2C_FT6_ADDR,0x03, buffer, 4);
				//tp_data.state = buffer[0];
				tp_data.x = (buffer[0]&0x0F)<<8 | buffer[1];
				tp_data.y = (buffer[2]&0x0F)<<8 | buffer[3];
				xQueueSend(ft6_data_queue, &tp_data, 1000);
			}
		}
	}
}

/*
static void ft6_read_reg(uint8_t reg, uint8_t *data, uint16_t size)
{
    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_FT6_ADDR << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_FT6_ADDR << 1) | I2C_MASTER_READ, 1);
    if (size > 1) {
        i2c_master_read(cmd, data, size - 1, I2C_MASTER_ACK);
    }
    if (size > 0) {
        i2c_master_read_byte(cmd, &data[size-1], I2C_MASTER_NACK);
    }
    i2c_master_stop(cmd);

    esp_err_t err = ESP_FAIL;

    err = i2c_master_cmd_begin(I2C_PORT, cmd, 1000000);
    i2c_cmd_link_delete(cmd);

    ESP_ERROR_CHECK(err);
}
*/

/*
static void ft6_write_reg(uint8_t reg, uint8_t *data, uint16_t size)
{
    i2c_cmd_handle_t write_cmd = i2c_cmd_link_create();
    i2c_master_start(write_cmd);
    i2c_master_write_byte(write_cmd, (I2C_FT6_ADDR << 1) | I2C_MASTER_WRITE, 1);
    i2c_master_write_byte(write_cmd, reg, 1);

    if (size > 0) {
        i2c_master_write(write_cmd, data, size, 1);
    }
    i2c_master_stop(write_cmd);

    esp_err_t err = ESP_FAIL;

    err = i2c_master_cmd_begin(I2C_PORT, write_cmd, 1000000);

    i2c_cmd_link_delete(write_cmd);

    ESP_ERROR_CHECK(err);
}
*/

void ft6_init()
{
	ft6_sem = xSemaphoreCreateBinary();

	ft6_data_queue = xQueueCreate(FT6_QUEUE_SIZE, sizeof (tp_coord_t));

	xTaskCreate(&ft6_task, "Touch Task", 1024*5, NULL, 1, NULL);


	gpio_config_t intr_config = {
			.intr_type = GPIO_INTR_NEGEDGE,
			.pin_bit_mask = 1ULL << EXT_FT6_PIN,
			.mode = GPIO_MODE_INPUT,
			.pull_up_en = 1,
	};

	ESP_ERROR_CHECK(gpio_config(&intr_config));
	ESP_ERROR_CHECK(gpio_install_isr_service(0));
	ESP_ERROR_CHECK(gpio_isr_handler_add(EXT_FT6_PIN, ft6_isr_handler, NULL));

	uint8_t buffer[1] = {0x00};
	i2c_bus_write(I2C_FT6_ADDR, 0xA4, buffer, 1);

}

void ft6_get_touch(uint16_t *x, uint16_t *y, uint16_t *state)
{}

QueueHandle_t ft6_get_queue()
{
	return ft6_data_queue;
}

