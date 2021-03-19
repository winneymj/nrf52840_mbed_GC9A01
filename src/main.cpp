/* mbed Microcontroller Library
 * Copyright (c) 2017-2019 ARM Limited
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdio.h>

#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "platform/NonCopyable.h"

#include "ble/GattServer.h"
#include "BLEProcess.h"
#include "Components/ble/CurrentTimeService.h"
#include "Components/ble/AlertNotificationService.h"
#include "Components/ble/NotificationManager.h"
#include "Components/datetime/DateTimeController.h"

#include <lvgl/lvgl.h>
#include <lv_drivers/display/GC9A01.h>
#include <bma423_main.h>
#include "WatchAPI.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

#define LVGL_TICK 5
// 5 milliseconds (5000 microseconds)
#define TICKER_TIME 1000 * LVGL_TICK

extern events::EventQueue app_queue;
events::EventQueue* queue = mbed_event_queue();

Thread t;

BLE &ble_interface{BLE::Instance()};
Mytime::Controllers::DateTimeController date_time_controller;
Mytime::Controllers::NotificationManager notification_manager;
Mytime::Controllers::CurrentTimeService current_time_service(date_time_controller);
Mytime::Controllers::AlertNotificationService alert_notification_service(notification_manager);
Mytime::Controllers::BLEProcess ble_process(*queue, ble_interface);
mbed::Callback<void(BLE&, events::EventQueue&)> post_init_cb[] = {
    callback(&current_time_service, &Mytime::Controllers::CurrentTimeService::start),
    callback(&alert_notification_service, &Mytime::Controllers::AlertNotificationService::start),
    NULL
};

lv_disp_buf_t disp_buf;
lv_color_t buf[LV_HOR_RES_MAX * 10];
lv_disp_drv_t disp_drv;

PwmOut VibMotor(P0_12);

// Declare button interrupts
InterruptIn button_RT(P0_3, PullUp); // Right Top
InterruptIn button_RM(P0_13, PullUp); // Right Middle
InterruptIn button_RB(P0_15, PullUp); // Right Bottom
InterruptIn button_LB(P0_25, PullUp); // Left Bottom

// LCD Light level pins
DigitalOut light_level1(P0_30, 0);
DigitalOut light_level2(P0_28, 0);
DigitalOut light_level3(P0_31, 0);

Ticker ticker;
static lv_obj_t *s_background_obj;

/**********************
 *  STATIC VARIABLES
 **********************/
/*Declare the "source code image" which is stored in the flash*/
LV_IMG_DECLARE(warning)

void lvl_ticker_func()
{
  // printf("lvl_ticker_func: ENTER \r\n");
  //Call lv_tick_inc(x) every x milliseconds in a Timer or Task (x should be between 1 and 10). 
  //It is required for the internal timing of LittlevGL.
  lv_tick_inc(LVGL_TICK);

  //Call lv_task_handler() periodically every few milliseconds. 
  //It will redraw the screen if required, handle input devices etc.  
  // lv_task_handler();
}

void button_RTop()
{
  SEGGER_RTT_printf(0, "button_RTop:!\n");
}

void button_RMiddle()
{
  SEGGER_RTT_printf(0, "button_RMiddle:!\n");
}

void button_RBottom()
{
  SEGGER_RTT_printf(0, "button_RBottom:!\n");
}

void button_LBottom()
{
  SEGGER_RTT_printf(0, "button_LBottom:!\n");
}

void button_init()
{
  SEGGER_RTT_printf(0, "button_init: ENTER\n");

  button_RT.fall(&button_RTop); // Setup interrupt callback
  button_RM.fall(&button_RMiddle); // Setup interrupt callback
  button_RB.fall(&button_RBottom); // Setup interrupt callback
  button_LB.fall(&button_LBottom); // Setup interrupt callback

  SEGGER_RTT_printf(0, "button_init: EXIT\n");
}

lv_area_t get_bounds(const lv_obj_t *obj) {
  lv_area_t cords_p;
  lv_obj_get_coords(obj, &cords_p);
  return cords_p;
}

void eventcb()
{
  // printf("eventcb()\r\n");
  //Call lv_task_handler() periodically every few milliseconds. 
  //It will redraw the screen if required, handle input devices etc.  
  lv_task_handler();
}

void notificationHandler()
{
  SEGGER_RTT_printf(0, "notificationHandler: ENTER\r\n");
  Mytime::Controllers::NotificationManager::Notification notif = notification_manager.GetLastNotification();
  SEGGER_RTT_printf(0, "\tnotificationHandler: size=%d\r\n", notif.message.size());

  SEGGER_RTT_printf(0, "\t");
  for (size_t i = 0; i < 10/*notif.message.size()*/; ++i)
  {
    SEGGER_RTT_printf(0, "%02X,", notif.message[i]);
  }
  SEGGER_RTT_printf(0, "\r\n");

  SEGGER_RTT_printf(0, "\tbreak watchface dispatch\r\n");
  app_queue.break_dispatch();

  SEGGER_RTT_printf(0, "notificationHandler: EXIT\r\n");
}


void window_load() {
  // Get default display
  lv_obj_t *screen = lv_scr_act();

  // Fill background with blue
  static lv_style_t style_screen;
  lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_obj_add_style(screen, LV_OBJ_PART_MAIN, &style_screen);

  // Get the bounds of the screen
  // lv_area_t bounds = get_bounds(screen);

  // Create background object base rectangle
  s_background_obj = lv_obj_create(screen, NULL);
  lv_obj_set_size(s_background_obj, 150, 150);
  lv_obj_align(s_background_obj, NULL, LV_ALIGN_IN_BOTTOM_MID, 0, 0);

  // Place it hidden off the bottom of the screen
  // lv_obj_set_pos(s_background_obj, bounds.x1, bounds.y2);

  // Fill base rectangle with yellow
  static lv_style_t style_background;
  lv_style_set_bg_color(&style_background, LV_STATE_DEFAULT, LV_COLOR_YELLOW);
  lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 3);
  lv_style_set_border_color(&style_background, LV_STATE_DEFAULT, LV_COLOR_BLACK);

  lv_obj_add_style(s_background_obj, LV_OBJ_PART_MAIN, &style_background);

  // Now add warning image
  lv_obj_t *icon = lv_img_create(s_background_obj, NULL);

  /*From variable*/
  lv_img_set_src(icon, &warning);

  // Set position
  lv_obj_align(icon, NULL, LV_ALIGN_IN_TOP_LEFT, 6, 6);

  // Create page style
  static lv_style_t style_page;
  lv_style_copy(&style_page, &style_background);
  lv_style_set_text_font(&style_page, LV_STATE_DEFAULT, &lv_font_montserrat_36);  /*Set a larger font*/
  lv_style_set_border_width(&style_page, LV_STATE_DEFAULT, 0);

  // Create a page
  lv_obj_t * page = lv_obj_create(s_background_obj, NULL);
  lv_obj_add_style(page, LV_OBJ_PART_MAIN, &style_page);
  lv_obj_align(page, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
  lv_obj_set_width(page, 150);
  lv_obj_set_height(page, 100);

  /*Create a label on the page*/
  lv_obj_t * label = lv_label_create(page, NULL);

  lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
  lv_obj_set_width(label, 150);          /*Set the label width to max value to not show hor. scroll bars*/
  lv_label_set_text(label, "Battery is low!\nConnect the charger.");
}

void init_ble()
{
    SEGGER_RTT_printf(0, "init_ble: ble_process.on_init()\r\n");
    ble_process.on_init(post_init_cb);

    // bind the event queue to the ble interface, initialize the interface
    // and start advertising
    SEGGER_RTT_printf(0, "init_ble: ble_process.start()\r\n");
    ble_process.start();
}

void lvgl_init()
{
    // Display graphics init
    lv_init();

    printf("main: lv_init() done\r\n");
    lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
    printf("main: lv_disp_buf_init() done\r\n");

    lv_disp_drv_init(&disp_drv);
    disp_drv.flush_cb = GC9A01_flush;
    disp_drv.buffer = &disp_buf;
    lv_disp_drv_register(&disp_drv);

    printf("main: lv_disp_drv_register() done\r\n");

    ticker.attach_us(mbed::callback(&lvl_ticker_func), TICKER_TIME);

    printf("main: ticker.attach() done\r\n");

    // Set callback for lv_task_handler to redraw the screen if necessary
    queue->call_every(5, mbed::callback(&eventcb));

    // window_load();
}

int main()
{
    SEGGER_RTT_printf(0, "main: START\r\n");

    // Initalize the display driver GC9A01
    GC9A01_init();

    // Initialize BLE
    init_ble();

    // Initialize the Accelerator
    BMA423_init();

    // Initialize button interrupts
    button_init();

    // Display graphics init
    lvgl_init();


    // When we get a notification we need to notify a method

	// GC9A01_fillScreen(WHITE_COLOUR);

    Mytime::Controllers::WatchAPI watchFace;
    t.start(mbed::callback(&watchFace, &Mytime::Controllers::WatchAPI::main));

    SEGGER_RTT_printf(0, "********** main: df() ***********\r\n");
    // Process the event queue.
    queue->dispatch_forever();

    return 0;
}

