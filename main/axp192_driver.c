/*
 * axp192_driver.c
 *
 *  Created on: 6 ago. 2022
 *      Author: JoseCarlos
 */
/*INCLUDE*************************************************************************/
#include "esp_log.h"

#include "axp192_driver.h"

/*INCLUDES********************************************************************/
#define AXP192		"AXP192"

/*PROTOTYPES**********************************************************************/
//static void axp192_write(uint8_t reg, uint8_t *data, uint16_t size);
static void axp192_write_bits(uint8_t reg, uint8_t data, uint8_t off, uint8_t size);
//static void axp192_read(uint8_t reg, uint8_t *data, uint8_t size);

/*FUNCTIONS**********************************************************************/
/*
static void axp192_write(uint8_t reg, uint8_t *data, uint16_t size)
{

    i2c_cmd_handle_t write_cmd = i2c_cmd_link_create();
    i2c_master_start(write_cmd);
    i2c_master_write_byte(write_cmd, (I2C_AXP_ADDR << 1) | I2C_MASTER_WRITE, 1);
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

static void axp192_write_bits(uint8_t reg, uint8_t data, uint8_t off, uint8_t size)
{
	uint8_t buffer;
	i2c_bus_read(I2C_AXP_ADDR, reg, &buffer, 1);

    buffer &= ~(((1 << size) - 1) << off);
    data &= (1 << size) - 1;
    buffer |= data << off;

    i2c_bus_write(I2C_AXP_ADDR, reg, &buffer, 1);
}

/*
static void axp192_read(uint8_t reg, uint8_t *data, uint8_t size)
{

    i2c_cmd_handle_t cmd = i2c_cmd_link_create();

	i2c_master_start(cmd);
	i2c_master_write_byte(cmd, (I2C_AXP_ADDR << 1) | I2C_MASTER_WRITE, 1);
	i2c_master_write_byte(cmd, reg, 1);

    i2c_master_start(cmd);
    i2c_master_write_byte(cmd, (I2C_AXP_ADDR << 1) | I2C_MASTER_READ, 1);
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

#ifndef BATERY_DEFAULT_CHARGE
#define BATERY_DEFAULT_CHARGE
#define BAT_DEF_OFF_VOLT		3000
#define BAT_DEF_CURRENT			CHARGE_Current_100mA
#define BAT_DEF_VOLT			CHARGE_VOLT_4200mV
#define BAT_ENA_CHAGE			1
#endif

void axp192_init()
{
	/*
    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = I2C_SDA,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_io_num = I2C_SCL,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = I2C_BAUD,
        // .clk_flags = 0,
    };
    ESP_ERROR_CHECK(i2c_param_config(I2C_PORT, &conf));
    ESP_ERROR_CHECK(i2c_driver_install(I2C_PORT, conf.mode, 0, 0, 0));
    */

	axp192_set_off_volt(BAT_DEF_OFF_VOLT);
	axp192_set_charge_current(BAT_DEF_CURRENT);
	axp192_set_charge_volt(BAT_DEF_VOLT);
	axp192_set_enable_charge(BAT_ENA_CHAGE);

}

/*GENERAL VOTLTAGE AND CURRENT BUS*/
/*
float axp192_get_bus_volt(power_bus_t bus)
{
	switch(bus)
	{
	case BA
	}
}
float axp192_get_bus_current(power_bus_t bus);
*/

/*BATERY*/
void axp192_set_off_volt(uint16_t m_volt)
{
	uint8_t value = (m_volt-AXP192_VOFF_VOLT_MIN)/AXP192_VOFF_VOLT_STEP;
	axp192_write_bits(AXP192_VOFF_VOLT_REG, value, 0, 3);
}

void axp192_set_charge_volt(charge_volt_t m_volt)
{
	axp192_write_bits(AXP192_CHG_CTL1_REG, m_volt, 5, 2);
}

void axp192_set_charge_current(chage_current_t m_ampe)
{
	axp192_write_bits(AXP192_CHG_CTL1_REG, m_ampe, 0, 4);
}

void axp192_set_enable_charge(uint8_t ena)
{
	axp192_write_bits(AXP192_CHG_CTL1_REG, ena, 7, 1);
}

float axp192_get_batery_in_current()
{
	uint8_t buffer[2];
	i2c_bus_read(I2C_AXP_ADDR, AXP192_BAT_ADC_CURRENT_IN_REG, buffer, 2);
	return (0.5)*((buffer[0] << 5) | (buffer[1] & 0x1F));
}

float axp192_get_batery_out_current()
{
	uint8_t buffer[2];
	i2c_bus_read(I2C_AXP_ADDR, AXP192_BAT_ADC_CURRENT_OUT_REG, buffer, 2);
	return (0.5)*((buffer[0] << 5) | (buffer[1] & 0x1F));
}

float axp192_get_batery_current()
{
	uint16_t c_in;
	uint16_t c_out;

	return axp192_get_batery_in_current()-axp192_get_batery_out_current();

}
float axp192_get_batery_volt()
{
	uint8_t buffer[2];
	i2c_bus_read(I2C_AXP_ADDR, AXP192_BAT_ADC_VOLTAGE_REG, buffer, 2);
	return (1.1 / 1000.0)*((buffer[0] << 4) | (buffer[1] & 0x0F));
}

/*OTHER VOLTAJE PORTS*/
float axp192_get_vbus_volt()
{
	uint8_t buffer[2];
	i2c_bus_read(I2C_AXP_ADDR, AXP192_VBUS_ADC_VOLTAGE_REG, buffer, 2);
	return (1.7 / 1000.0)*((buffer[0] << 4) | (buffer[1] & 0x0F));
}

float axp192_get_acin_volt()
{
	uint8_t buffer[2];
	i2c_bus_read(I2C_AXP_ADDR, AXP192_ACIN_ADC_VOLTAGE_REG, buffer, 2);
	return (1.7 / 1000.0)*((buffer[0] << 4) | (buffer[1] & 0x0F));
}

/*POWER ON/OFF*/
void apx192_set_time_off(time_off_t time_off)
{
	axp192_write_bits(AXP192_PEK_CTL_REG, time_off, 0, 2);
}

void axp192_set_time_on(time_on_t time_on)
{
	axp192_write_bits(AXP192_PEK_CTL_REG, time_on, 6, 2);
}

/*GPIO*/
void axp192_set_gpio_mode(axp192_gpio_t gpio, axp192_gpio_mode_t mode)
{
	switch(gpio)
	{
	case GPIO_1:
		axp192_write_bits(AXP192_GPIO1_CTL_REG, mode, 0, 3);
		break;
	case GPIO_2:
		axp192_write_bits(AXP192_GPIO2_CTL_REG, mode, 0, 3);
		break;
	case GPIO_4:
		/*Is not the same case in diferent ports*/
		switch(mode)
		{
		case GPIO_MODE_OPENDRAIN:
			mode = 0x01;
			break;
		default:
			break;
		}
		axp192_write_bits(AXP192_GPIO34_CTL_REG, mode, 2, 2);
		axp192_write_bits(AXP192_GPIO34_CTL_REG, 1, 7, 1);
		break;
	default:
	break;
	}
}

void axp192_set_gpio_level(axp192_gpio_t gpio, uint8_t level)
{
	switch(gpio)
	{
	case GPIO_1:
		axp192_write_bits(AXP192_GPIO012_STATE_REG, level, 1, 1);
	break;
	case GPIO_2:
		axp192_write_bits(AXP192_GPIO012_STATE_REG, level, 2, 1);
	break;
	case GPIO_4:
		axp192_write_bits(AXP192_GPIO34_STATE_REG, level, 1, 1);
	break;

	default:
		break;
	}
}

void axp192_set_ext(uint8_t enable)
{
	axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, 1, AXP192_EXT_EN_BIT, 1);
}

/*POWER OUTPUTS*/
//LDO
void axp192_set_ldo_volt(axp192_ldo_t ldo, uint16_t m_volt)
{
    uint8_t value = (m_volt - AXP192_LDO_VOLT_MIN) / AXP192_LDO_VOLT_STEP;

    switch(ldo)
    {
    case LDO_2:
    	axp192_write_bits(AXP192_LDO23_VOLT_REG, value, 4, 4);
    	break;
    case LDO_3:
    	axp192_write_bits(AXP192_LDO23_VOLT_REG, value, 0, 4);
    	break;
    default:
    	break;
    }
}

void axp192_set_ldo_enable(axp192_ldo_t ldo, uint8_t ena)
{
    switch(ldo)
    {
    case LDO_2:
    	axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, ena, AXP192_LDO2_EN_BIT, 1);
    	break;
    case LDO_3:
    	axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, ena, AXP192_LDO3_EN_BIT, 1);
    	break;
    default:
    	break;
    }
}

//DCDC
void axp192_set_dcdc_volt(axp192_dcdc_t dcdc, uint16_t m_volt)
{
	uint8_t value = (m_volt - AXP192_DC_VOLT_MIN) / AXP192_DC_VOLT_STEP;

	/*
	switch(dcdc)
	{
	case DCDC_1:
		axp192_write(AXP192_DC1_VOLT_REG, &value, 1);
		break;

	case DCDC_2:
		axp192_write(AXP192_DC2_VOLT_REG, &value, 1);
		break;

	case DCDC_3:
		axp192_write(AXP192_DC3_VOLT_REG, &value, 1);
		break;
	default:
		break;
	}
	*/

	i2c_bus_write(I2C_AXP_ADDR, dcdc, &value, 1);

}

void axp192_set_dcdc_enable(axp192_dcdc_t dcdc, uint8_t ena)
{
	switch(dcdc)
	{
	case DCDC_1:
		axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, ena, AXP192_DC1_EN_BIT, 1);
		break;

	case DCDC_2:
		axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, ena, AXP192_DC2_EN_BIT, 1);
		break;

	case DCDC_3:
		axp192_write_bits(AXP192_LDO23_DC123_EXT_CTL_REG, ena, AXP192_DC3_EN_BIT, 1);
		break;
	default:
		break;
	}
}

