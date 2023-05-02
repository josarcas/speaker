/*
 * a2dp_bt_driver.c
 *
 *  Created on: 9 abr. 2023
 *      Author: JoseArratia
 */
/*INCLUDES*******************************************/
#include "a2dp_bt_driver.h"

#include "string.h"


/*DEFINES********************************************/

/*GLOBAL VARIABLES***********************************/

/*PROTOTYPES*****************************************/

/*FUNCTIONS******************************************/

void a2dp_init()
{
	ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_BLE));

	esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();

	esp_bt_controller_init(&bt_cfg);
	esp_bt_controller_enable(ESP_BT_MODE_CLASSIC_BT);
	esp_bluedroid_init();
	esp_bluedroid_enable();

	/*Paring*/
	esp_bt_sp_param_t param_type = ESP_BT_SP_IOCAP_MODE;
	esp_bt_io_cap_t iocap = ESP_BT_IO_CAP_IO;
	esp_bt_gap_set_security_param(param_type, &iocap, sizeof(uint8_t));

    esp_bt_pin_type_t pin_type = ESP_BT_PIN_TYPE_FIXED;
    esp_bt_pin_code_t pin_code;

    memcpy(pin_code, A2DP_PIN, 4);
    esp_bt_gap_set_pin(pin_type, 4, pin_code);

    esp_bt_dev_set_device_name(A2DP_NAME);
}



