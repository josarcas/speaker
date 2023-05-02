/*
 * ns4168_driver.c
 *
 *  Created on: 10 dic. 2022
 *      Author: JoseArratia
 */
/*INCLUDES***********************************************************************************************/
#include "ns4168_driver.h"

#include "driver/i2s_std.h"
#include "driver/gpio.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

/*GLOBAL VARIABLES**************************************************************************************/
static i2s_chan_handle_t                tx_chan;        // I2S tx channel handler
static i2s_chan_handle_t                rx_chan;        // I2S rx channel handler

/*FUNCTIONS**********************************************************************************************/
void ns4_init()
{
    i2s_chan_config_t chan_cfg = I2S_CHANNEL_DEFAULT_CONFIG(I2S_NUM_AUTO, I2S_ROLE_MASTER);
    ESP_ERROR_CHECK(i2s_new_channel(&chan_cfg, &tx_chan, &rx_chan));

    i2s_std_config_t std_cfg = {
        .clk_cfg  = I2S_STD_CLK_DEFAULT_CONFIG(SAMPLE_RATE),
        .slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_MONO),
        .gpio_cfg = {
            .mclk = -1,
            .bclk = I2S_BCLK_PIN,
            .ws   = I2S_LRCK_PIN,
            .dout = I2S_SADTA_PIN,
            .din  = I2S_SIDTA_PIN,
            .invert_flags = {
                .mclk_inv = false,
                .bclk_inv = false,
                .ws_inv   = false,
            },
        },
    };

    std_cfg.slot_cfg.slot_mask = I2S_STD_SLOT_RIGHT;
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(tx_chan, &std_cfg));
    ESP_ERROR_CHECK(i2s_channel_init_std_mode(rx_chan, &std_cfg));

    ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
//	axp192_set_gpio_mode(SPK_EN, GPIO_MODE_OPENDRAIN);
//	axp192_set_gpio_level(SPK_EN, 0);
}

esp_err_t ns4_set_config(uint32_t s_rate, i2s_data_bit_width_t s_bits, i2s_slot_mode_t mode)
{
    i2s_std_clk_config_t clk_cfg = I2S_STD_CLK_DEFAULT_CONFIG(s_rate);
    i2s_std_slot_config_t slot_cfg = I2S_STD_MSB_SLOT_DEFAULT_CONFIG(s_bits, mode);

	ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
	ESP_ERROR_CHECK(i2s_channel_reconfig_std_clock(tx_chan, &clk_cfg));
	ESP_ERROR_CHECK(i2s_channel_reconfig_std_slot(tx_chan, &slot_cfg));
	ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));

	return ESP_OK;//i2s_set_clk(I2S_NUM_AUTO, s_rate, s_bits, );
}

void ns4_enable(uint8_t ena)
{
	axp192_set_gpio_level(SPK_EN, ena);
}


void ns4_write_buffer(const uint8_t *buffer, uint32_t size)
{
	size_t bytes_w;
	//ESP_ERROR_CHECK(i2s_channel_enable(tx_chan));
    ////ESP_ERROR_CHECK(i2s_channel_enable(rx_chan));
	ESP_ERROR_CHECK(i2s_channel_write(tx_chan, buffer, size, &bytes_w, 10000));
	//ESP_ERROR_CHECK(i2s_channel_disable(tx_chan));
}



