/*
 * lv_esp_port.c
 *
 *  Created on: 20 dic. 2022
 *      Author: JoseArratia
 */
/*INCLUDES*********************************************************/
#include "esp_lcd_ili9341.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "esp_err.h"
#include "esp_log.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

#include "lv_esp_port.h"
#include "ft6336u_driver.h"

/*DEFINES**********************************************************/
#define LV_ESP_PORT_PART			"LV_ESP"

//#define FS_CURRENT_IMG				"ESPIOS/i.bin"

//COLOR BUFFER
#define C_BUFFER_SIZE				MAX_BUFFER_FILE-(MAX_BUFFER_FILE%3);

//CUSTOM IMAGE OFFSET
#define IMG_C_OFF			0x00//Color format
#define IMG_W_OFF			0x01//Weigth offset (2 bytes)
#define IMG_H_OFF			0x03//Height offset (2 bytes)
#define IMG_D_OFF			0x04//Data offset (Weight*height*2)

/*GLOBAL VARIABLES*************************************************/

//LCD
static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_disp_drv_t disp_drv;
static lv_disp_draw_buf_t disp_buf;
static lv_disp_t *disp;

//TOUCH PAD
static lv_indev_drv_t tp_drv;
static QueueHandle_t tp_queue;
static tp_coord_t tp_coord;

//FILE SYSTEM
//static FILE *g_file = NULL;
char image_id = 0x2F;					//Current image id
char image_file[MAX_LENGH_NAME_FILE];


/*PROTOTYPES*****************************************************/
//LCD
static bool lcd_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
		esp_lcd_panel_io_event_data_t *edata, void *user_ctx);
static void lcd_lvgl_set();
static void lvgl_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area,
		lv_color_t *color_map);
static void lvgl_port_update_callback(lv_disp_drv_t *drv);
static void lvgl_systick_handler(void *arg);
static void sys_lcd_set();

//TOUCH PAD
static void lv_touch_callback(lv_indev_drv_t * drv, lv_indev_data_t * data);

/*FUNCTIONS*******************************************************/
static bool lcd_notify_lvgl_flush_ready(esp_lcd_panel_io_handle_t panel_io,
		esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    lv_disp_drv_t *disp_driver = (lv_disp_drv_t *)user_ctx;
    lv_disp_flush_ready(disp_driver);
    return false;
}

static void lcd_lvgl_set()
{
	ESP_LOGI(LV_ESP_PORT_PART, "INITIALIZE LVGL");

	lv_init();

    lv_color_t *buf1 = heap_caps_malloc(LCD_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf1);
    lv_color_t *buf2 = heap_caps_malloc(LCD_HEIGHT * 20 * sizeof(lv_color_t), MALLOC_CAP_DMA);
    assert(buf2);

    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_HEIGHT * 20);

    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_HEIGHT;
    disp_drv.ver_res = LCD_WIDTH;
    disp_drv.flush_cb = lvgl_flush_callback;			//Upadate screen
    disp_drv.drv_update_cb = lvgl_port_update_callback;	//Update screen orientation
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;
    disp = lv_disp_drv_register(&disp_drv);

    //Set time base for lvgl
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &lvgl_systick_handler,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, LVGL_TICK_PERIOD_MS * 1000));
}

//UPDATE SCREEN for especific window
static void lvgl_flush_callback(lv_disp_drv_t *drv, const lv_area_t *area,
		lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsetx2 = area->x2;
    int offsety1 = area->y1;
    int offsety2 = area->y2;
    esp_lcd_panel_draw_bitmap(panel_handle, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);
}

//Update screen orientation
static void lvgl_port_update_callback(lv_disp_drv_t *drv)
{
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t) drv->user_data;

    switch (drv->rotated)
    {
    case LV_DISP_ROT_NONE:
        esp_lcd_panel_swap_xy(panel_handle, false);
        break;
    case LV_DISP_ROT_90:
        esp_lcd_panel_swap_xy(panel_handle, true);
        break;
    case LV_DISP_ROT_180:
        esp_lcd_panel_swap_xy(panel_handle, false);
        break;
    case LV_DISP_ROT_270:
        esp_lcd_panel_swap_xy(panel_handle, true);
        break;
    }
}

static void lvgl_systick_handler(void *arg)
{
	lv_tick_inc(LVGL_TICK_PERIOD_MS);
	lv_timer_handler();
}

static void sys_lcd_set()
{
	ESP_LOGI(LV_ESP_PORT_PART, "Init LCD device");

    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = LCD_DC_PIN,
        .cs_gpio_num = LCD_CS_PIN,
        .pclk_hz = LCD_PIXEL_CLOCK,
        .lcd_cmd_bits = LCD_CMD_BITS,
        .lcd_param_bits = LCD_PARAM_BITS,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .on_color_trans_done = lcd_notify_lvgl_flush_ready,
        .user_ctx = &disp_drv,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)SYS_SPI_HOST, &io_config, &io_handle));

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = -1,					//Reset by AXTP
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16,
    };

    ESP_ERROR_CHECK(esp_lcd_new_panel_ili9341(io_handle, &panel_config, &panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    esp_lcd_panel_swap_xy(panel_handle, false);
    esp_lcd_panel_mirror(panel_handle, false, false);
}

//TOUCH PAD
static void lv_touch_callback(lv_indev_drv_t * drv, lv_indev_data_t * data)
{
	if(xQueueReceive(tp_queue, &tp_coord, 1)==pdTRUE)
	{
		data->point.x = tp_coord.x;
		data->point.y = tp_coord.y;
		data->state = LV_INDEV_STATE_PRESSED;
	}
	else
		data->state = LV_INDEV_STATE_RELEASED;
}

/**********************************************************************************************/
/****************************************HEADER FUNCTIONS**************************************/
/**********************************************************************************************/
//LCD
void lv_esp_init()
{
	axp192_set_gpio_mode(LCD_RST_PIN, GPIO_MODE_OPENDRAIN);

	axp192_set_dcdc_volt(LCD_BRIGTHNESS, 2700);
	axp192_set_dcdc_enable(LCD_BRIGTHNESS, 1);

	lv_esp_reset();
	sys_lcd_set();
	lcd_lvgl_set();
}

void lv_esp_reset()
{
	axp192_set_gpio_level(LCD_RST_PIN, 1);
	vTaskDelay(50/portTICK_PERIOD_MS);
	axp192_set_gpio_level(LCD_RST_PIN, 0);
	vTaskDelay(50/portTICK_PERIOD_MS);
	axp192_set_gpio_level(LCD_RST_PIN, 1);
	vTaskDelay(50/portTICK_PERIOD_MS);
}


//TOUCH PAD
void lv_esp_touch_init()
{
	ft6_init();
	tp_queue =  ft6_get_queue();
}

void lv_esp_touch_set()
{
	lv_indev_drv_init(&tp_drv);
	tp_drv.type = LV_INDEV_TYPE_POINTER;
	tp_drv.disp = disp;
	tp_drv.read_cb = lv_touch_callback;
	tp_drv.user_data = NULL;

	if(lv_indev_drv_register(&tp_drv)== NULL)
		ESP_LOGE(LV_ESP_PORT_PART, "Error set touch pad");
}
