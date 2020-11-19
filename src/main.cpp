/* mbed Microcontroller Library
 * Copyright (c) 2018 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */

#include "mbed.h"
// #include "USBConsole.h"
#include <lvgl/lvgl.h>
#include <lv_drivers/display/GC9A01.h>
#include <BMA42X-Sensor-Driver/BMA423.h>
// enum etick_units {
//   SECOND_UNIT
// };

// typedef struct {
//   Callback<void()> init_handler;
//   struct tick_info {
//     Callback<void()> tick_handler;
//     etick_units tick_units;
//   };
// } PebbleAppHandler;

#define LVGL_TICK 5
// 5 milliseconds (5000 microseconds)
#define TICKER_TIME 1000 * LVGL_TICK

#define INSET 5
#define HOURS_RADIUS 10
#ifndef M_PI
#define M_PI           3.14159265358979323846
#endif

static int32_t sHours = 4;
static int32_t sMinutes = 45;

Ticker ticker;
PwmOut VibMotor(P0_12);

/**********************
 *  STATIC VARIABLES
 **********************/
/*Declare the "source code image" which is stored in the flash*/
LV_IMG_DECLARE(warning)


static int32_t getAngleForHour(int hour) {
  return (hour * 360) / 12;
}

static int32_t getAngleForMinutes(int minutes) {
  return (minutes * 360) / 60;
}

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

void eventcb() {
  // printf("eventcb()\r\n");
  //Call lv_task_handler() periodically every few milliseconds. 
  //It will redraw the screen if required, handle input devices etc.  
  lv_task_handler();
}

//---------------------------------------------------------------------------------------
// WATCHFACE

lv_point_t pointFromPolar(lv_area_t &bounds, int angleInDegrees) {
  lv_point_t retVal;

  int w = lv_area_get_width(&bounds);
  int h = lv_area_get_height(&bounds);
  int radius = w / 2;
  lv_coord_t cx = bounds.x1 + (w / 2);
  lv_coord_t cy = bounds.y1 + (h / 2);
  // printf("pointFromPolar angleInDegrees=%d\r\n", angleInDegrees);
  // printf("pointFromPolar radius=%d\r\n", radius);
  // printf("pointFromPolar bounds=%d,%d,%d,%d\r\n", bounds.x1, bounds.y1, bounds.x2, bounds.y2);

  // // Convert from degrees to radians via multiplication by PI/180        
  // float x = (float)(radius * cos(angleInDegrees * M_PI / 180.0)) + bounds.x1;
  // float y = (float)(radius * sin(angleInDegrees * M_PI / 180.0)) + bounds.y1;
  // Convert from degrees to radians via multiplication by PI/180        
  float x = (float) cx + (radius * cos(angleInDegrees / (180.0 / M_PI)));
  float y = (float) cy + (radius * sin(angleInDegrees / (180.0 / M_PI)));
  // printf("pointFromPolar x=%f\r\n", x);
  // printf("pointFromPolar y=%f\r\n", y);
  retVal.x = (lv_coord_t)x;
  retVal.y = (lv_coord_t)y;
  return retVal;
}

lv_area_t bounds_inset(const lv_area_t &bounds, const lv_coord_t offset) {
  lv_area_t retVal = {bounds.x1 + offset, bounds.y1 + offset, bounds.x2 - offset, bounds.y2 - offset};
  return retVal;
}

lv_area_t get_bounds(const lv_obj_t *obj) {
  lv_area_t cords_p;
  lv_obj_get_coords(obj, &cords_p);
  return cords_p;
}

static lv_obj_t *s_background_obj;
#define BOUNDS_WIDTH(obj) (obj.x2 - obj.x1 + 1)
#define BOUNDS_HEIGHT(obj) (obj.y2 - obj.y1 + 1) 

void screen_colour() {
static bool cntr = false;

  static lv_style_t style_screen;
  if (cntr) {
    lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_BLACK);
  } else {
    lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_WHITE);
  }
  cntr = !cntr;
  
  // Get default display
  lv_obj_t *screen = lv_scr_act();
  lv_obj_add_style(screen, LV_OBJ_PART_MAIN, &style_screen);
}

void window_load() {
  // Get default display
  lv_obj_t *screen = lv_scr_act();

  // Fill background with blue
  static lv_style_t style_screen;
  lv_style_set_bg_color(&style_screen, LV_STATE_DEFAULT, LV_COLOR_BLUE);
  lv_obj_add_style(screen, LV_OBJ_PART_MAIN, &style_screen);

  // Get the bounds of the screen
  lv_area_t bounds = get_bounds(screen);

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
  lv_style_set_text_font(&style_page, LV_STATE_DEFAULT, &lv_font_montserrat_18);  /*Set a larger font*/
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

// void pebble_circle_watchface(void) {
//   // Fill background with blue
//   static lv_style_t style_screen;
//   lv_style_copy(&style_screen, &lv_style_plain);
//   style_screen.body.main_color = LV_COLOR_BLUE;
//   style_screen.body.grad_color = LV_COLOR_BLUE;  // Comment this out to get a graduated blue->white
//   lv_obj_set_style(lv_scr_act(), &style_screen);

//   // Get bounds
//   lv_area_t bounds = layer_get_bounds(lv_scr_act());

//   // 12 hours only with maximim size
//   sHours -= (sHours > 12) ? 12 : 0;

//   // Minutes are expanding circle arc
//   int minuteAngle = getAngleForMinutes(sMinutes);
//   lv_area_t frame = bounds_inset(bounds, (4 * INSET));

//   /*Create style for the Arcs*/
//   static lv_style_t style;
//   lv_style_copy(&style, &lv_style_plain);
//   style.line.color = LV_COLOR_MAKE(0x88, 0x8A, 0xD3); /*Arc color These are 888 numbers */
//   style.line.width = 20;                       /*Arc width*/
//   printf("style.line.color.full=0x%hx\r\n", style.line.color.full);

//   /*Create an Arc*/
//   lv_obj_t * arc = lv_arc_create(lv_scr_act(), NULL);
//   lv_arc_set_style(arc, LV_ARC_STYLE_MAIN, &style);          /*Use the new style*/
//   lv_arc_set_angles(arc, minuteAngle - 180, 0);
//   lv_obj_set_size(arc, frame.x2, frame.y2);
//   lv_obj_align(arc, NULL, LV_ALIGN_CENTER, 0, 0);

//   /*Create style for the dots*/
//   static lv_style_t styleBlack;
//   lv_style_copy(&styleBlack, &lv_style_plain);
//   styleBlack.body.main_color = LV_COLOR_BLACK;
//   styleBlack.body.grad_color = LV_COLOR_BLACK;
//   styleBlack.body.radius = LV_RADIUS_CIRCLE;

//   static lv_style_t styleWhite;
//   lv_style_copy(&styleWhite, &styleBlack);
//   styleWhite.body.main_color = LV_COLOR_WHITE;
//   styleWhite.body.grad_color = LV_COLOR_WHITE;

//   // create new smaller bounds object using the style margin
//   lv_area_t newBounds = bounds_inset(bounds, (2.2 * HOURS_RADIUS));

//   // Hours are dots
//   for (int i = 0; i < 12; i++) {
//     int hourPos = getAngleForHour(i);
//     lv_point_t pos = pointFromPolar(newBounds, hourPos);
//     // printf("pointFromPolar = %d, %d\r\n", pos.x, pos.y);

//     // Create object to draw
//     lv_obj_t *btn = lv_obj_create(lv_scr_act(), NULL);

//     lv_obj_set_style(btn, i <= sHours ? &styleWhite : &styleBlack);
//     lv_obj_set_size(btn, HOURS_RADIUS, HOURS_RADIUS);
//     lv_obj_set_pos(btn, pos.x - (HOURS_RADIUS / 2), pos.y - (HOURS_RADIUS / 2));
//   }

//   lv_obj_t *btn2 = lv_obj_create(lv_scr_act(), NULL);
//   lv_obj_set_style(btn2, &styleBlack);
//   lv_obj_set_size(btn2, HOURS_RADIUS, HOURS_RADIUS);
//   lv_obj_set_pos(btn2, 115, 5);
// }

void lv_ex_page_1(void)
{
    // /*Create a scroll bar style*/
    // static lv_style_t style_bg;
    // lv_style_copy(&style_bg, &lv_style_plain);
    // style_bg.body.border.color = LV_COLOR_BLACK;
    // style_bg.body.border.width = 1;
    // static lv_style_t style_scrl;
    // lv_style_copy(&style_scrl, &lv_style_plain);
    // style_scrl.body.border.color = LV_COLOR_BLACK;
    // // style_bg.body.main_color = LV_COLOR_BLACK;
    // // style_bg.body.grad_color = LV_COLOR_BLACK;
    // // static lv_style_t style_sb;
    // // lv_style_copy(&style_sb, &lv_style_pretty);
    // // static lv_style_t style_sb;
    // // lv_style_copy(&style_sb, &lv_style_scr);
    // // style_sb.body.main_color = LV_COLOR_BLACK;
    // // style_sb.body.grad_color = LV_COLOR_BLACK;
    // // style_sb.body.border.color = LV_COLOR_WHITE;
    // // style_sb.body.border.width = 1;
    // // style_sb.body.border.opa = LV_OPA_70;
    // // style_sb.body.radius = 30;
    // // style_sb.body.opa = LV_OPA_60;
    // // style_sb.body.padding.right = 3;
    // // style_sb.body.padding.bottom = 3;
    // // style_sb.body.padding.inner = 8;        /*Scrollbar width*/

    // /*Create a page*/
    // lv_obj_t * page = lv_page_create(lv_scr_act(), NULL);
    // lv_obj_set_size(page, 150, 150);
    // lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);
    // lv_page_set_style(page, LV_PAGE_STYLE_BG, &style_bg);           /*Set the scrollbar style*/
    // lv_page_set_style(page, LV_PAGE_STYLE_SCRL, &style_scrl);           /*Set the scrollbar style*/

    // /*Create a label on the page*/
    // lv_obj_t * label = lv_label_create(page, NULL);
    // lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
    // lv_obj_set_width(label, lv_page_get_fit_width(page));          /*Set the label width to max value to not show hor. scroll bars*/
    // lv_label_set_text(label, "this is a long piece of text to try out how the label text is displayed\n"
    //                          "wonder how well it works?");
}

//--------------------------------------------------------------------------------------

// main() runs in its own thread in the OS
int main()
{
  printf("main: ENTER\r\n");

  // float brightness = 0.5f;
  // float addition = 0.001;
  // VibMotor.period(0.010f); // 1 second period

  // while(true) {
    // Initalize the display driver GC9A01
    GC9A01_init();

    // Initialize the Accelerator
    BMA423_init();

    // Do a led pwm bright to dim and to bright START
    // VibMotor.write(brightness); // 50% duty cycle

    // brightness += addition;

    // if (brightness <= 0.0) {
    //   addition = 0.0001;
    // }
    // if (brightness >= 1.0) {
    //   addition = -0.0001;
    // }
    // wait_us(500);
    // Do a led pwm bright to dim and to bright END
  // }

  printf("main: GC9A01_init() done\r\n");

  lv_init();

  printf("main: lv_init() done\r\n");
  static lv_disp_buf_t disp_buf;
  static lv_color_t buf[LV_HOR_RES_MAX * 10];
  lv_disp_buf_init(&disp_buf, buf, NULL, LV_HOR_RES_MAX * 10);
  printf("main: lv_disp_buf_init() done\r\n");

  lv_disp_drv_t disp_drv;
  lv_disp_drv_init(&disp_drv);
  disp_drv.flush_cb = GC9A01_flush;
  disp_drv.buffer = &disp_buf;
  lv_disp_drv_register(&disp_drv);

	// GC9A01_fillScreen(WHITE_COLOUR);

  printf("main: lv_disp_drv_register() done\r\n");

  ticker.attach_us(callback(&lvl_ticker_func), TICKER_TIME);

	printf("main: ticker.attach() done\r\n");
  events::EventQueue queue;

  // Set callback for lv_task_handler to redraw the screen if necessary
  queue.call_every(5, callback(&eventcb));

  window_load();

  // // Set callback for lv_task_handler to redraw the screen if necessary
  // queue.call_every(1000, callback(&screen_colour));

  // lv_ex_page_1();


  queue.dispatch_forever();
}
