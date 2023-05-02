/*
 * espios_bt.h
 *
 *  Created on: 9 abr. 2023
 *      Author: JoseArratia
 */

#ifndef MAIN_ESPIOS_BT_H_
#define MAIN_ESPIOS_BT_H_
/*INCLUDES*******************************************/
#include "a2dp_bt_driver.h"

#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*DEFINES*********************************************/
#define BT_ID		 "BLUETOOTH"

#ifndef MD_TRACK_QUEUE						//META DATA TRACK
#define MD_TRACK_QUEUE
#define MD_TRACK_SIZE						sizeof(t_metadata_t)
#define MD_TRACK_BUFF						1
#endif


/*PROTOTYPES*****************************************/
void init_a2dp_prof();

void a2dp_get_metadata(t_metadata_t *data);
void a2dp_set_volume(uint8_t volume);
void a2dp_send_cmd(esp_avrc_pt_cmd_t cmd);

QueueHandle_t trank_reg_md();

#endif /* MAIN_ESPIOS_BT_H_ */
