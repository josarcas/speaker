/*
 * axp192_driver.h
 *
 *  Created on: 6 ago. 2022
 *      Author: JoseCarlos
 */

#ifndef MAIN_AXP192_DRIVER_H_
#define MAIN_AXP192_DRIVER_H_
/*INCLUDES********************************************************************/
//#include "stdint.h"
//#include "driver/i2c.h"
#include "i2c_bus_manager.h"

#include "esp_err.h"

/*DEFINES**********************************************************************/
#ifndef I2C_SYS_BUS
#define I2C_SYS_BUS
#define I2C_PORT		I2C_NUM_1
#define I2C_BAUD		400000
#define I2C_SDA			21
#define I2C_SCL			22
#endif

#define I2C_AXP_ADDR	0x34

#ifndef AXP192_REG
#define AXP192_REG
#define AXP192_DC_VOLT_STEP  25
#define AXP192_DC_VOLT_MIN   700
#define AXP192_DC_VOLT_MAX   3500

#define AXP192_LDO_VOLT_STEP 100
#define AXP192_LDO_VOLT_MIN  1800
#define AXP192_LDO_VOLT_MAX  3300

#define AXP192_VOFF_VOLT_STEP 100
#define AXP192_VOFF_VOLT_MIN  2600
#define AXP192_VOFF_VOLT_MAX  3300

#define AXP192_LDO23_DC123_EXT_CTL_REG 0x12
#define AXP192_DC1_EN_BIT   (0)
#define AXP192_DC3_EN_BIT   (1)
#define AXP192_LDO2_EN_BIT  (2)
#define AXP192_LDO3_EN_BIT  (3)
#define AXP192_DC2_EN_BIT   (4)
#define AXP192_EXT_EN_BIT   (6)

#define AXP192_DC1_VOLT_REG         0x26
#define AXP192_DC2_VOLT_REG         0x23
#define AXP192_DC3_VOLT_REG         0x27
#define AXP192_LDO23_VOLT_REG       0x28
#define AXP192_VBUS_IPSOUT_CTL_REG  0x30 // not support yet
#define AXP192_VOFF_VOLT_REG        0x31 // PWRON short press in here
#define AXP192_POWEROFF_REG         0x32 // CHGLED in here
#define AXP192_CHG_CTL1_REG         0x33
#define AXP192_CHG_CTL2_REG         0x34
#define AXP192_SPARE_CHG_CTL_REG    0x35
#define AXP192_PEK_CTL_REG          0x36
#define AXP192_CHG_BOOL_REG         0x01

#define AXP192_ADC1_ENABLE_REG      0x82
#define BAT_VOLT_BIT        (7)
#define BAT_CURRENT_BIT     (6)
#define ACIN_VOLT_BIT       (5)
#define ACIN_CURRENT_BIT    (4)
#define VBUS_VOLT_BIT       (3)
#define VBUS_CURRENT_BIT    (2)
#define APS_VOLT_BIT        (1)
#define TS_BIT              (0)

#define AXP192_ACIN_ADC_VOLTAGE_REG         0x56
#define AXP192_ACIN_ADC_CURRENT_REG         0x58

#define AXP192_VBUS_ADC_VOLTAGE_REG         0x5A
#define AXP192_VBUS_ADC_CURRENT_REG         0x5C

#define AXP192_BAT_ADC_VOLTAGE_REG          0x78
#define AXP192_BAT_ADC_CURRENT_IN_REG       0x7A
#define AXP192_BAT_ADC_CURRENT_OUT_REG      0x7C

#define AXP192_GPIO0_CTL_REG                0x90
#define AXP192_GPIO0_VOLT_REG               0x91
#define AXP192_GPIO1_CTL_REG                0x92
#define AXP192_GPIO2_CTL_REG                0x93
#define AXP192_GPIO34_CTL_REG               0x95

#define AXP192_GPIO34_STATE_REG             0x96
#define AXP192_GPIO012_STATE_REG            0x94
#endif

/*TYPEDEFS********************************************************************/
/*GENERAL VOTLTAGE AND CURRENT BUS*/
/*
typedef enum{
	BATERY_BUS,
	VOLTAJE_BUS,
	ACIN_BUS,
}power_bus_t;
*/

/*BATERY*/
typedef enum{
    CHARGE_VOLT_4100mV = 0b0000,
    CHARGE_VOLT_4150mV = 0b0001,
    CHARGE_VOLT_4200mV = 0b0010,
    CHARGE_VOLT_4360mV = 0b0011,
}charge_volt_t;

typedef enum{
    CHARGE_Current_100mA = 0b0000,
    CHARGE_Current_190mA,
    CHARGE_Current_280mA,
    CHARGE_Current_360mA,
    CHARGE_Current_450mA,
    CHARGE_Current_550mA,
    CHARGE_Current_630mA,
    CHARGE_Current_700mA,
    CHARGE_Current_780mA,
    CHARGE_Current_880mA,
    CHARGE_Current_960mA,
    CHARGE_Current_1000mA,
    CHARGE_Current_1080mA,
    CHARGE_Current_1160mA,
    CHARGE_Current_1240mA,
    CHARGE_Current_1320mA,
}chage_current_t;

/*POWER ON/OFF*/
typedef enum{
    OFF_4S 		= 0x00,
    OFF_6S 		= 0x01,
    OFF_8S 		= 0x02,
    OFF_10S 	= 0x03,
}time_off_t;

typedef enum{
    ON_128mS 	= 0x00,
    ON_512mS 	= 0x01,
    ON_1S 		= 0x02,
    ON_2S		= 0x03,
}time_on_t;

/*GPIO*/
typedef enum{
	GPIO_0,
	GPIO_1,
	GPIO_2,
	GPIO_3,
	GPIO_4
}axp192_gpio_t;

typedef enum{
	/*	Note: This values only apply on GPIO1, but in the set function the values
	 * 	are changed.
	 */
	GPIO_MODE_OPENDRAIN			= 0x00,
	GPIO_MODE_UINPUT			= 0x01,
	GPIO_MODE_PWM				= 0x10,
}axp192_gpio_mode_t;

/*POWER OUTPUTS*/
//LDO
typedef enum{
	LDO_0,
	LDO_1,
	LDO_2,
	LDO_3
}axp192_ldo_t;
//DCDC
typedef enum{
	DCDC_1				= AXP192_DC1_VOLT_REG,
	DCDC_2				= AXP192_DC2_VOLT_REG,
	DCDC_3				= AXP192_DC3_VOLT_REG
}axp192_dcdc_t;

/*PROTOTYPES******************************************************************/
void axp192_init();

/*GENERAL VOTLTAGE AND CURRENT BUS*/
//float axp192_get_bus_volt(power_bus_t bus);
//float axp192_get_bus_current(power_bus_t bus);

/*BATERY*/
void axp192_set_off_volt(uint16_t m_volt);
void axp192_set_charge_volt(charge_volt_t m_volt);
void axp192_set_charge_current(chage_current_t m_ampe);
void axp192_set_enable_charge(uint8_t ena);

float axp192_get_batery_in_current();
float axp192_get_batery_out_current();
float axp192_get_batery_current();
float axp192_get_batery_volt();

/*OTHER VOLTAJE PORTS*/
float axp192_get_vbus_volt();
float axp192_get_acin_volt();

/*POWER ON/OFF*/
void apx192_set_time_off(time_off_t time_off);
void axp192_set_time_on(time_on_t time_on);

/*GPIO*/
void axp192_set_gpio_mode(axp192_gpio_t gpio, axp192_gpio_mode_t mode);
void axp192_set_gpio_level(axp192_gpio_t gpio, uint8_t level);
void axp192_set_ext(uint8_t enable);

/*POWER OUTPUTS*/
//LDO
void axp192_set_ldo_volt(axp192_ldo_t ldo, uint16_t m_volt);
void axp192_set_ldo_enable(axp192_ldo_t ldo, uint8_t ena);
//DCDC
void axp192_set_dcdc_volt(axp192_dcdc_t dcdc, uint16_t m_volt);
void axp192_set_dcdc_enable(axp192_dcdc_t dcdc, uint8_t ena);



#endif /* MAIN_AXP192_DRIVER_H_ */
