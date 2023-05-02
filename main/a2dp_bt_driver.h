/*
 * a2dp_bt_driver.h
 *
 *  Created on: 9 abr. 2023
 *      Author: JoseArratia
 */

#ifndef MAIN_A2DP_BT_DRIVER_H_
#define MAIN_A2DP_BT_DRIVER_H_
/*INCLUDES********************************************/
#include "esp_bt.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gap_bt_api.h"
#include "esp_a2dp_api.h"
#include "esp_avrc_api.h"

/*DEFINES**********************************************/
#define A2DP_NAME					"ESPIOS"
#define A2DP_PIN					"1234"

#ifndef METADATA_ATTR
#define METADATA_ATTR
#define MAX_METADATA_LEN			50
#define METAD_TITLE_ID				1
#define METAD_ARTIST_ID				2
#define METAD_ALBUM					4
#endif
/*TYPEDEFS*********************************************/
typedef struct{
	char title[MAX_METADATA_LEN];
	char artist[MAX_METADATA_LEN];
	char album[MAX_METADATA_LEN];
}t_metadata_t;

/*PROTOTYPES*******************************************/
void a2dp_init();



#endif /* MAIN_A2DP_BT_DRIVER_H_ */
