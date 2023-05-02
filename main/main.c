/*INCLUDES******************************************************/
#include "espios.h"

#include "espios_bt.h"

#include "music_icon.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

/*DEFINES********************************************************/
#define BG_COLOR				0x181818
#define SECOND_COLOR			0x8C9EA7

#define T_SLIDER_ID				0
#define PP_BUTTON_ID			1
#define PREV_BUTTON_ID			2
#define NEXT_BUTTON_ID			3

/*GLOBAL VARIABLES*****************************/
//SYSTEM

//SemaphoreHandle_t screen_mutex = NULL;

//MUSIC GUI
/*Slider*/
lv_obj_t *t_slider =NULL;
lv_obj_t *v_slider =NULL;

/*Button*/
lv_obj_t *pp_b = NULL;
lv_obj_t *prev_b = NULL;
lv_obj_t *next_b = NULL;

/*Label*/
lv_obj_t *title_l = NULL;
lv_obj_t *artist_l = NULL;
lv_obj_t *t_curr = NULL;
lv_obj_t *t_max = NULL;
lv_obj_t *pp_l = NULL;

uint8_t track_status = 0;

/*PROTOTYPES************************************/
//SYSTEM TASK

//MUSIC GUI
void gui_gen_s_time();
void gui_gen_l_track();
void gui_gen_b_track();

/*FUNCTIONS*************************************/
static void event_handler(lv_event_t *event)
{
	uint8_t *obj_id = (uint8_t *)event->user_data;

	switch(*obj_id)
	{
	case T_SLIDER_ID:
		break;
	case PP_BUTTON_ID:
		if(track_status)
		{
			a2dp_send_cmd(ESP_AVRC_PT_CMD_PAUSE);
			lv_label_set_text(pp_l, LV_SYMBOL_PLAY);

		}
		else
		{
			a2dp_send_cmd(ESP_AVRC_PT_CMD_PLAY);
			lv_label_set_text(pp_l, LV_SYMBOL_PAUSE);
		}
		track_status = !track_status;

		break;
	case PREV_BUTTON_ID:
		a2dp_send_cmd(ESP_AVRC_PT_CMD_BACKWARD);
		break;
	case NEXT_BUTTON_ID:
		a2dp_send_cmd(ESP_AVRC_PT_CMD_FORWARD);
		break;
	default:
		break;
	}

}

//MUSIC GUI
void gui_gen_s_time()
{
    /*Create a time stamp slider*/
	t_slider = lv_slider_create(lv_scr_act());
    lv_obj_align(t_slider, LV_ALIGN_CENTER, 0, 20);
    static uint8_t obj_id = T_SLIDER_ID;
    lv_obj_add_event_cb(t_slider, event_handler, LV_EVENT_RELEASED, &obj_id);
	lv_obj_set_style_bg_color(t_slider, lv_color_hex(SECOND_COLOR), 0);
	lv_obj_set_content_height(t_slider, 5);
	static lv_style_t style_indicator;
    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_white());
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_obj_add_style(t_slider, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(t_slider, &style_indicator, LV_PART_KNOB);

}
void gui_gen_l_track()
{
    /*Print title*/
	title_l = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(title_l, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(title_l, lv_color_white(), 0);
	lv_obj_align(title_l, LV_ALIGN_CENTER, 0, -30);
	lv_label_set_text(title_l, "");

    /*Print artist*/
	artist_l = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(artist_l, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(artist_l, lv_color_hex(SECOND_COLOR), 0);
	lv_obj_align(artist_l, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(artist_l, "");

	/*Current time*/
	t_curr = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(t_curr, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(t_curr, lv_color_hex(SECOND_COLOR), 0);
	lv_obj_align(t_curr, LV_ALIGN_BOTTOM_LEFT, 0, -70);
	lv_label_set_text(t_curr, "0:00");

	/*Max time*/
	t_max = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(t_max, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(t_max, lv_color_hex(SECOND_COLOR), 0);
	lv_obj_align(t_max, LV_ALIGN_BOTTOM_RIGHT, 0, -70);
	lv_label_set_text(t_max, "0:00");
}

void gui_gen_b_track()
{
	/*Play pause button*/
	pp_b = lv_btn_create(lv_scr_act());
	static uint8_t pp_id = PP_BUTTON_ID;
	lv_obj_add_event_cb(pp_b, event_handler, LV_EVENT_PRESSED, &pp_id);
	lv_obj_align(pp_b, LV_ALIGN_CENTER, 0, 80);
	vTaskDelay(2000/portTICK_PERIOD_MS);
	pp_l = lv_label_create(pp_b);
	lv_obj_set_style_bg_color(pp_b, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(pp_l, lv_color_white(), 0);
	lv_label_set_text(pp_l, LV_SYMBOL_PLAY);

	/*Pevious button*/
	prev_b = lv_btn_create(lv_scr_act());
	static uint8_t prev_id = PREV_BUTTON_ID;
	lv_obj_add_event_cb(prev_b, event_handler, LV_EVENT_PRESSED, &prev_id);
	lv_obj_align(prev_b, LV_ALIGN_CENTER, -60, 80);
	vTaskDelay(2000/portTICK_PERIOD_MS);
	static lv_obj_t * prev_l = NULL;
	prev_l = lv_label_create(prev_b);
	lv_obj_set_style_bg_color(prev_b, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(prev_l, lv_color_white(), 0);
	lv_label_set_text(prev_l, LV_SYMBOL_PREV);

	/*Next button*/
	next_b = lv_btn_create(lv_scr_act());
	static uint8_t next_id = NEXT_BUTTON_ID;
	lv_obj_add_event_cb(next_b, event_handler, LV_EVENT_PRESSED, &next_id);
	lv_obj_align(next_b, LV_ALIGN_CENTER, 60, 80);
	vTaskDelay(2000/portTICK_PERIOD_MS);
	static lv_obj_t * next_l = NULL;
	next_l = lv_label_create(next_b);
	lv_obj_set_style_bg_color(next_b, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(next_l, lv_color_white(), 0);
	lv_label_set_text(next_l, LV_SYMBOL_NEXT);

}

void espios_music(void *args)
{
	t_metadata_t data;
	QueueHandle_t md_queue;
	QueueHandle_t status;
	esp_avrc_playback_stat_t d_status;

	md_queue = track_reg_md();
	status = track_reg_status();

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(BG_COLOR), 0);

	vTaskDelay(1000/portTICK_PERIOD_MS);
	gui_gen_s_time();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	gui_gen_l_track();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	gui_gen_b_track();

    for(;;)
    {
    	if(xQueueReceive(md_queue, &data, 1) == pdTRUE)
    	{
    		lv_label_set_text(title_l, data.title);
    		lv_label_set_text(artist_l, data.artist);
    		ESP_LOGI("BT", "Track time %s", data.play_time);
    	}
    	if(xQueueReceive(status, &d_status, 1) == pdTRUE)
    	{
    		switch(d_status)
    		{
    		case ESP_AVRC_PLAYBACK_PLAYING:
    			lv_label_set_text(pp_l, LV_SYMBOL_PAUSE);
    			break;
    		case ESP_AVRC_PLAYBACK_PAUSED:
    			lv_label_set_text(pp_l, LV_SYMBOL_PLAY);
    			break;
    		default:
    			break;
    		}
    	}
    }

}

/*MAIN***************************************/

void app_main()
{
	espios_init();
	init_a2dp_prof();
	vTaskDelay(1000/portTICK_PERIOD_MS);
	lv_obj_clean(lv_scr_act());
	xTaskCreate(espios_music, "music", 1024*5, NULL, 1, NULL);
}

