/*INCLUDES******************************************************/
#include "espios.h"

#include "espios_bt.h"

/*DEFINES********************************************************/
#define BG_COLOR				0x181818
#define SECOND_COLOR			0x8C9EA7

/*GLOBAL VARIABLES*****************************/
//SYSTEM

//SemaphoreHandle_t screen_mutex = NULL;

/*PROTOTYPES************************************/
//SYSTEM TASK

//MUSIC GUI
void gui_gen_s_time();
void gui_gen_l_track();

/*FUNCTIONS*************************************/
lv_obj_t * slider =NULL;
static void event_handler(lv_event_t *event)
{
	a2dp_set_volume(lv_slider_get_value(slider));
	ESP_LOGI("Event", "Volumen %ld", lv_slider_get_value(slider));
}

void espios_music(void *args)
{
	t_metadata_t data;

	lv_obj_set_style_bg_color(lv_scr_act(), lv_color_hex(BG_COLOR), 0);

    /*Create a time stamp slider*/
    slider = lv_slider_create(lv_scr_act());
    //lv_slider_set_range(slider, 0, 100);
    lv_obj_align(slider, LV_ALIGN_CENTER, 0, 60);
    lv_obj_add_event_cb(slider, event_handler, LV_EVENT_RELEASED, NULL);
//    lv_obj_set_width(slider, 10);
//    lv_obj_set_content_height(slider, 100);
	lv_obj_set_style_bg_color(slider, lv_color_hex(SECOND_COLOR), 0);
	lv_obj_set_content_height(slider, 5);

	static lv_style_t style_indicator;
    lv_style_init(&style_indicator);
    lv_style_set_bg_opa(&style_indicator, LV_OPA_COVER);
    lv_style_set_bg_color(&style_indicator, lv_color_white());
    lv_style_set_radius(&style_indicator, LV_RADIUS_CIRCLE);
    lv_obj_add_style(slider, &style_indicator, LV_PART_INDICATOR);
    lv_obj_add_style(slider, &style_indicator, LV_PART_KNOB);

    /*Print title*/
	lv_obj_t *title_l = NULL;
	title_l = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(title_l, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(title_l, lv_color_white(), 0);
	lv_obj_align(title_l, LV_ALIGN_CENTER, 0, -30);
	lv_label_set_text(title_l,"");

    /*Print artist*/
	lv_obj_t *artist_l = NULL;
	artist_l = lv_label_create(lv_scr_act());
	lv_obj_set_style_bg_color(artist_l, lv_color_hex(BG_COLOR), 0);
	lv_obj_set_style_text_color(artist_l, lv_color_white(), 0);
	lv_obj_align(artist_l, LV_ALIGN_CENTER, 0, 0);
	lv_label_set_text(artist_l,"");

    for(;;)
    {
    	a2dp_get_metadata(&data);
    	lv_label_set_text(title_l, data.title);
    	lv_label_set_text(artist_l, data.artist);
    	vTaskDelay(30000/portTICK_PERIOD_MS);
    	a2dp_set_pos();
    	ESP_LOGI("EVENT", "POSChangue");
    }

}

/*MAIN***************************************/

void app_main()
{
	espios_init();
	init_a2dp_prof();
	lv_obj_clean(lv_scr_act());
	xTaskCreate(espios_music, "music", 1024*3, NULL, 1, NULL);
}

