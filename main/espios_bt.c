/*
 * espios_bt.c
 *
 *  Created on: 9 abr. 2023
 *      Author: JoseArratia
 */
/*INCLUDES*******************************************************/
#include "espios_bt.h"

#include "esp_err.h"
#include "esp_log.h"
#include "esp_system.h"


#include "ns4168_driver.h"
#include "axp192_driver.h"
#include "string.h"

/*DEFINES********************************************/
#define REQ_DATA		ESP_AVRC_MD_ATTR_TITLE|ESP_AVRC_MD_ATTR_ARTIST|ESP_AVRC_MD_ATTR_ALBUM

#ifndef CAP_MASK
#define CAP_MASK
#define PLAY_STATUS			0x01<<1
#define TRACK_CHANGE		0x01<<2
#define REACHED_END			0x01<<3
#define REACHED_START		0x01<<4
#define POS_CHANGE			0x01<<5
#define BAT_STATUS			0x01<<6
#define SYS_STATUS_CHG		0x01<<7
#define PLAYER_APP_SET		0x01<<8
#define NOW_PLAYING_c		0x01<<9
#define AVAIL_PLAYERS		0x01<<10
#define ADDRESSED_PLAYER	0x01<<11
#define UID					0x01<<12
#define VOLUME_CHANGE		0x01<<13
#endif

#define DEVICE_CAP			REACHED_END|PLAY_STATUS|TRACK_CHANGE|REACHED_START|POS_CHANGE|VOLUME_CHANGE

/*GLOBAL VARIABLES************************************************/
static esp_avrc_rn_evt_cap_mask_t s_avrc_peer_rn_cap;

static uint8_t abs_volume;

/*Meta data*/
static t_metadata_t track_meta;
static QueueHandle_t md_track_queue = NULL;

/*PROTOTYPES*****************************************************/
/*Track info*/
static void request_tk_md();
static void request_pb_cd();
static void request_pos_cd();

/*Callbacks*/
static void avr_event_cb(uint8_t event_id, esp_avrc_rn_param_t *event_parameter);
static void a2d_event_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param);
static void avr_ctrl_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param);
static void gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param);
static void a2d_skin_data_cb(const uint8_t *data, uint32_t len);

/*FUNCTIONS******************************************************/
static void request_tk_md()
{
    esp_avrc_ct_send_metadata_cmd(1, REQ_DATA);

    /* register notification if peer support the event_id */
    if (esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_TEST, &s_avrc_peer_rn_cap,
                                           ESP_AVRC_RN_TRACK_CHANGE)) {
        esp_avrc_ct_send_register_notification_cmd(2, ESP_AVRC_RN_TRACK_CHANGE, 0);
    }
}

static void request_pb_cd()
{
    if (esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_TEST, &s_avrc_peer_rn_cap,
                                           ESP_AVRC_RN_PLAY_STATUS_CHANGE)) {
        esp_avrc_ct_send_register_notification_cmd(3, ESP_AVRC_RN_PLAY_STATUS_CHANGE, 0);
    }
}

static void request_pos_cd()
{
    if (esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_TEST, &s_avrc_peer_rn_cap,
                                           ESP_AVRC_RN_PLAY_POS_CHANGED)) {
        esp_avrc_ct_send_register_notification_cmd(4, ESP_AVRC_RN_PLAY_POS_CHANGED, 10);
    }
}

static void avr_event_cb(uint8_t event_id, esp_avrc_rn_param_t *event_parameter)
{
    switch (event_id) {
    /* when new track is loaded, this event comes */
    case ESP_AVRC_RN_TRACK_CHANGE:
    	request_tk_md();
        break;
    /* when track status changed, this event comes */
    case ESP_AVRC_RN_PLAY_STATUS_CHANGE:

        break;
    /* when track playing position changed, this event comes */
    case ESP_AVRC_RN_PLAY_POS_CHANGED:
        break;
    /* others */
    default:
        break;
    }
}

static void a2d_event_cb(esp_a2d_cb_event_t event, esp_a2d_cb_param_t *param)
{
	 switch(event)
	 {
	   case ESP_A2D_CONNECTION_STATE_EVT:
	   {
	       if(param->conn_stat.state == ESP_A2D_CONNECTION_STATE_DISCONNECTED)
	       {
	           esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);
	       }
	       else if(param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTED)
	       {
	           esp_bt_gap_set_scan_mode(ESP_BT_NON_CONNECTABLE, ESP_BT_NON_DISCOVERABLE);
	       }
	       else if (param->conn_stat.state == ESP_A2D_CONNECTION_STATE_CONNECTING)
	       {
	       }
	       break;
	    }
	    /* when audio stream transmission state changed, this event comes */
	    case ESP_A2D_AUDIO_STATE_EVT:
	    {
	    	switch(param->audio_stat.state)
	    	{
	    	case ESP_A2D_AUDIO_STATE_REMOTE_SUSPEND:
	    		ns4_enable(0);
	    		break;
	    	case ESP_A2D_AUDIO_STATE_STOPPED:
	    		break;
	    	case ESP_A2D_AUDIO_STATE_STARTED:
	    		ns4_enable(1);
	    		break;
	    	default:
	    		break;
	    	}

	        break;
	    }
	    /* when audio codec is configured, this event comes */
	    case ESP_A2D_AUDIO_CFG_EVT:
	    {
	        if (param->audio_cfg.mcc.type == ESP_A2D_MCT_SBC)
	        {
	            int sample_rate = 16000;
	            int ch_count = 2;
	            char oct0 = param->audio_cfg.mcc.cie.sbc[0];
	            if (oct0 & (0x01 << 6)) {
	                sample_rate = 32000;
	            } else if (oct0 & (0x01 << 5)) {
	                sample_rate = 44100;
	            } else if (oct0 & (0x01 << 4)) {
	                sample_rate = 48000;
	            }

	            if (oct0 & (0x01 << 3)) {
	                ch_count = 1;
	            }
	            ns4_set_config(sample_rate, I2S_DATA_BIT_WIDTH_16BIT, ch_count);
	            ESP_LOGI(BT_ID, "Sampling: %d, Mode: %s", sample_rate, ch_count==2? "stereo":"mono");
	        }
	    	break;

	    }
	    /* when a2dp init or deinit completed, this event comes */
	    case ESP_A2D_PROF_STATE_EVT:
	    {
	        break;
	    }
	    /* When protocol service capabilities configured, this event comes */
	    case ESP_A2D_SNK_PSC_CFG_EVT:
	    {

	        break;
	    }
	    /* when set delay value completed, this event comes */
	    case ESP_A2D_SNK_SET_DELAY_VALUE_EVT:
	    {

	        break;
	    }
	    /* when get delay value completed, this event comes */
	    case ESP_A2D_SNK_GET_DELAY_VALUE_EVT:
	    {
	    	esp_a2d_sink_set_delay_value(param->a2d_get_delay_value_stat.delay_value + 50);
	    	break;
	    }
	    /* others */
	    default:
	        break;
	 }
}

static void avr_ctrl_cb(esp_avrc_ct_cb_event_t event, esp_avrc_ct_cb_param_t *param)
{

    switch (event) {
    /* when connection state changed, this event comes */
    case ESP_AVRC_CT_CONNECTION_STATE_EVT:
    {
        if (param->conn_stat.connected)
        {
            /* get remote supported event_ids of peer AVRCP Target */
            esp_avrc_ct_send_get_rn_capabilities_cmd(0);
        }
        else
        	s_avrc_peer_rn_cap.bits = 0;

        break;
    }
    /* when passthrough responsed, this event comes */
    case ESP_AVRC_CT_PASSTHROUGH_RSP_EVT:
    {
    	 ESP_LOGI("BT CTRL", "AVRC passthrough rsp: key_code 0x%x, key_state %d", param->psth_rsp.key_code, param->psth_rsp.key_state);
        break;
    }
    /* when metadata responsed, this event comes */
    case ESP_AVRC_CT_METADATA_RSP_EVT:
    {
    	ESP_LOGI(BT_ID, "ID: %X, Metadata: %s", param->meta_rsp.attr_id, param->meta_rsp.attr_text);
        switch(param->meta_rsp.attr_id)
        {
        case METAD_TITLE_ID:
        	memcpy(track_meta.title, param->meta_rsp.attr_text, param->meta_rsp.attr_length);
        	track_meta.title[param->meta_rsp.attr_length] = 0;
        	break;
        case METAD_ARTIST_ID:
        	memcpy(track_meta.artist, param->meta_rsp.attr_text, param->meta_rsp.attr_length);
        	track_meta.artist[param->meta_rsp.attr_length] = 0;
        	break;
        case METAD_ALBUM:
        	memcpy(track_meta.album, (char *)param->meta_rsp.attr_text, param->meta_rsp.attr_length);
        	track_meta.album[param->meta_rsp.attr_length] = 0;
        	if(md_track_queue != NULL)
        		xQueueSend(md_track_queue, &track_meta, 1);
        	break;
        default:
        	break;
        }
    	break;
    }
    /* when notified, this event comes */
    case ESP_AVRC_CT_CHANGE_NOTIFY_EVT:
    {
    	avr_event_cb(param->change_ntf.event_id, &param->change_ntf.event_parameter);
        break;
    }
    /* when feature of remote device indicated, this event comes */
    case ESP_AVRC_CT_REMOTE_FEATURES_EVT:
    {
        break;
    }
    /* when notification capability of peer device got, this event comes */
    case ESP_AVRC_CT_GET_RN_CAPABILITIES_RSP_EVT:
    {
    	s_avrc_peer_rn_cap.bits = param->get_rn_caps_rsp.evt_set.bits;
        request_tk_md();
        request_pb_cd();
        request_pos_cd();
    	break;
    }
    /* others */
    default:
        break;
    }
}

static void avr_target_cb(esp_avrc_tg_cb_event_t event, esp_avrc_tg_cb_param_t *param)
{

    switch (event)
    {
    /* when connection state changed, this event comes */
    case ESP_AVRC_TG_CONNECTION_STATE_EVT:
    {
        break;
    }
    /* when passthrough commanded, this event comes */
    case ESP_AVRC_TG_PASSTHROUGH_CMD_EVT:
    {
        break;
    }
    /* when absolute volume command from remote device set, this event comes */
    case ESP_AVRC_TG_SET_ABSOLUTE_VOLUME_CMD_EVT:
    {
    	abs_volume = param->set_abs_vol.volume;
        break;
    }
    /* when notification registered, this event comes */
    case ESP_AVRC_TG_REGISTER_NOTIFICATION_EVT:
    {
        if (param->reg_ntf.event_id == ESP_AVRC_RN_VOLUME_CHANGE)
        {
            esp_avrc_rn_param_t rn_param;
            rn_param.volume = abs_volume;
            esp_avrc_tg_send_rn_rsp(ESP_AVRC_RN_VOLUME_CHANGE, ESP_AVRC_RN_RSP_INTERIM, &rn_param);
        }
        break;
    }
    /* when feature of remote device indicated, this event comes */
    case ESP_AVRC_TG_REMOTE_FEATURES_EVT:
    {
        break;
    }
    /* others */
    default:
        break;
    }
}

static void gap_cb(esp_bt_gap_cb_event_t event, esp_bt_gap_cb_param_t *param)
{
    switch (event) {
    /* when authentication completed, this event comes */
    case ESP_BT_GAP_AUTH_CMPL_EVT:
    {

        break;
    }
    /* when Security Simple Pairing user confirmation requested, this event comes */
    case ESP_BT_GAP_CFM_REQ_EVT:
    	esp_bt_gap_ssp_confirm_reply(param->cfm_req.bda, true);
        break;
    /* when Security Simple Pairing passkey notified, this event comes */
    case ESP_BT_GAP_KEY_NOTIF_EVT:
        break;
    /* when Security Simple Pairing passkey requested, this event comes */
    case ESP_BT_GAP_KEY_REQ_EVT:
        break;
    /* when GAP mode changed, this event comes */
    case ESP_BT_GAP_MODE_CHG_EVT:
        break;
    /* others */
    default: {
        break;
    }
    }
}

static void a2d_skin_data_cb(const uint8_t *data, uint32_t len)
{
	ns4_write_buffer(data, len);
}

void init_a2dp_prof()
{
	a2dp_init();

	esp_bt_gap_register_callback(gap_cb);

	ESP_ERROR_CHECK(esp_avrc_ct_init());
	esp_avrc_ct_register_callback(avr_ctrl_cb);

	ESP_ERROR_CHECK(esp_avrc_tg_init());
	esp_avrc_tg_register_callback(avr_target_cb);

//	esp_avrc_rn_evt_cap_mask_t cap ={
//			.bits = DEVICE_CAP};
//	esp_avrc_tg_set_rn_evt_cap(&cap);

    esp_avrc_rn_evt_cap_mask_t evt_set = {0};
    esp_avrc_rn_evt_bit_mask_operation(ESP_AVRC_BIT_MASK_OP_SET, &evt_set, ESP_AVRC_RN_VOLUME_CHANGE);
    ESP_ERROR_CHECK(esp_avrc_tg_set_rn_evt_cap(&evt_set));

    ESP_ERROR_CHECK(esp_a2d_sink_init());
    esp_a2d_register_callback(a2d_event_cb);
    esp_a2d_sink_register_data_callback(a2d_skin_data_cb);

    esp_a2d_sink_get_delay_value();

    esp_bt_gap_set_scan_mode(ESP_BT_CONNECTABLE, ESP_BT_GENERAL_DISCOVERABLE);

}

void a2dp_get_metadata(t_metadata_t *data)
{
	memcpy(data, &track_meta, sizeof(t_metadata_t));
}

void a2dp_set_volume(uint8_t volume)
{
    esp_avrc_rn_param_t rn_param;
    rn_param.volume = abs_volume = volume&0x7f;
    esp_avrc_tg_send_rn_rsp(ESP_AVRC_RN_VOLUME_CHANGE, ESP_AVRC_RN_RSP_CHANGED, &rn_param);
}

void a2dp_send_cmd(esp_avrc_pt_cmd_t cmd)
{
	esp_avrc_ct_send_passthrough_cmd(0, cmd, ESP_AVRC_PT_CMD_STATE_PRESSED);
}

QueueHandle_t trank_reg_md()
{
	if(md_track_queue == NULL)
		md_track_queue = xQueueCreate(MD_TRACK_BUFF, MD_TRACK_SIZE);
	return md_track_queue;
}

