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

#ifndef __WINDOW_API_H__
#define __WINDOW_API_H__

#include "mbed.h"
#include "Api.h"

#include <lvgl/lvgl.h>

extern "C"{
  #include "SEGGER_RTT.h"
}

#define TextLayer lv_obj_t

class GRect: lv_area_t
{
public:
    GRect(lv_coord_t x1, lv_coord_t y1, lv_coord_t x2, lv_coord_t y2)
    {
        _area.x1 = x1;
        _area.y1 = y1;
        _area.x2 = x2;
        _area.y2 = y2;
    };
    lv_coord_t x1() { return _area.x1; };
    lv_coord_t y1() { return _area.y1; };
    lv_coord_t x2() { return _area.x2; };
    lv_coord_t y2() { return _area.y2; };
    lv_coord_t width() { return _area.x2 - _area.x1; };
    lv_coord_t height() { return _area.y2 - _area.y1; };

private:
    lv_area_t _area;
};

/**
WindowHandler load
Called when the window is pushed to the screen when it's not loaded. This is a good moment to do the layout of the window.

WindowHandler appear
Called when the window comes on the screen (again). E.g. when second-top-most window gets revealed (again) after popping the top-most window, but also when the window is pushed for the first time. This is a good moment to start timers related to the window, or reset the UI, etc.

WindowHandler disappear
Called when the window leaves the screen, e.g. when another window is pushed, or this window is popped. Good moment to stop timers related to the window.

WindowHandler unload
Called when the window is deinited, but could be used in the future to free resources bound to windows that are not on screen.
**/

namespace Mytime {
    namespace Windows {

enum TimeUnits {
    SECOND_UNIT = 1000,
    MINUTE_UNIT = 60000,
    HOUR_UNIT = 36000,
    DAY_UNIT,
    MONTH_UNIT,
    YEAR_UNIT
};

class Window;

typedef struct {
    mbed::Callback<void(Mytime::Windows::Window*)> load{nullptr};
    mbed::Callback<void(void)> appear{nullptr};
    mbed::Callback<void(void)> disappear{nullptr};
    mbed::Callback<void(void)> unload{nullptr};
} WindowHandlers;

        /**
         */
        class Window
        {
        public:
            ~Window() {};
            Window(lv_obj_t* w) : _window(w) {};
            void setHandlers(WindowHandlers handlers)
            {
                _handlers = handlers;
            };

            WindowHandlers& getHandlers()
            {
                return _handlers;
            };

            lv_obj_t *getWindow() { return _window; };

        private:
            WindowHandlers _handlers;
            lv_obj_t * _window;
        };
    }
}

std::array<Mytime::Windows::Window*, 100> windows_array;

void window_set_window_handlers(Mytime::Windows::Window* w, Mytime::Windows::WindowHandlers handlers)
{
    SEGGER_RTT_printf(0, "window_set_window_handlers START\n\r");                 
    w->setHandlers(handlers);
    SEGGER_RTT_printf(0, "window_set_window_handlers EXIT\n\r");                 
}

Mytime::Windows::Window* window_create()
{
    SEGGER_RTT_printf(0, "window_create START\n\r");                 

    // Get default display
    lv_obj_t *screen = lv_scr_act();

    // Create background object base rectangle
    lv_obj_t *obj = lv_obj_create(screen, NULL);
    lv_obj_set_size(obj, 249, 249);
    lv_obj_align(obj, NULL, LV_ALIGN_CENTER, 0, 0);

    // Fill base rectangle with white
    static lv_style_t style_background;
    lv_style_set_bg_color(&style_background, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 1);
    // lv_style_set_border_color(&style_background, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    Mytime::Windows::Window* w = new Mytime::Windows::Window(obj);

    SEGGER_RTT_printf(0, "window_create EXIT\n\r");
    return w;
}

void window_destroy(Mytime::Windows::Window* w)
{
    lv_obj_clean(w->getWindow());    
}

TextLayer* text_layer_create(Mytime::Windows::Window *parent_window, GRect &rect)
{
    // Create page style
    static lv_style_t style_page;
    lv_style_set_bg_color(&style_page, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    lv_style_set_border_width(&style_page, LV_STATE_DEFAULT, 0);
    // lv_style_set_border_color(&style_page, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // lv_style_copy(&style_page, &style_background);
    // lv_style_set_text_font(&style_page, LV_STATE_DEFAULT, &lv_font_montserrat_18);  /*Set a larger font*/
    // lv_style_set_border_width(&style_page, LV_STATE_DEFAULT, 0);

    // Create a page
    lv_obj_t* page = lv_obj_create(parent_window->getWindow(), NULL);
    lv_obj_add_style(page, LV_OBJ_PART_MAIN, &style_page);
    // lv_obj_align(page, NULL, LV_ALIGN_IN_TOP_MID, 0, 40);
    lv_obj_set_width(page, rect.width());
    lv_obj_set_height(page, rect.height());
    lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);

    /*Create a label on the page*/
    lv_obj_t* label = lv_label_create(page, NULL);

    return label;
}

void window_stack_push(Mytime::Windows::Window* w, bool animate = false)
{
    SEGGER_RTT_printf(0, "window_stack_push START\n\r");                 
    // Push to the top of the stack and call the load handlers
    windows_array[0] = w;
    Mytime::Windows::WindowHandlers handlers = w->getHandlers();
    if (handlers.load)
    {
        handlers.load(w);
    }
    SEGGER_RTT_printf(0, "window_stack_push EXIT\n\r");                 
}

Mytime::Windows::Window* window_stack_pop(bool animated)
{
    SEGGER_RTT_printf(0, "window_stack_pop START\n\r");
    // Pop off and call handlers    
    Mytime::Windows::Window* w = windows_array[0];
    Mytime::Windows::WindowHandlers handlers = w->getHandlers();
    if (handlers.unload)
    {
        handlers.unload();
    }
    SEGGER_RTT_printf(0, "window_stack_pop EXIT\n\r");
    return w;
}

void text_layer_set_background_color(TextLayer *tl, lv_color_t color)
{
    // Fill base rectangle with yellow
    static lv_style_t style_background;
    lv_style_init(&style_background);

    lv_style_set_bg_color(&style_background, LV_STATE_DEFAULT, color);
    lv_obj_add_style(tl, LV_OBJ_PART_MAIN, &style_background);
}

void text_layer_set_text_color(TextLayer *tl, lv_color_t color)
{
    // Fill base rectangle with yellow
    static lv_style_t style_text_color;
    lv_style_init(&style_text_color);

    lv_style_set_text_color(&style_text_color, LV_STATE_DEFAULT, color);
    lv_obj_add_style(tl, LV_OBJ_PART_MAIN, &style_text_color);
}

void text_layer_set_text(TextLayer *tl, const char *txt)
{
    lv_label_set_text(tl, txt);
}

void text_layer_set_font(TextLayer *tl, const lv_font_t *fnt)
{
    static lv_style_t style_set_font;
    lv_style_init(&style_set_font);

    lv_style_set_text_font(&style_set_font, LV_STATE_DEFAULT, fnt);
    lv_obj_add_style(tl, LV_OBJ_PART_MAIN, &style_set_font);
}

void text_layer_set_text_alignment(TextLayer *tl, lv_align_t align)
{
    lv_obj_align(tl, NULL, align, 0, 0);
}

extern events::EventQueue event_queue;

static int tick_instance = -1;

void intermediate_ticker(mbed::Callback<void(struct tm *, Mytime::Windows::TimeUnits)> handler, Mytime::Windows::TimeUnits time_unit)
{
    SEGGER_RTT_printf(0, "intermediate_ticker START\n\r");
    time_t seconds = time(NULL);
    struct tm *current_time;
    current_time = localtime(&seconds);

    handler(current_time, time_unit);
    SEGGER_RTT_printf(0, "intermediate_ticker EXIT\n\r");
}

void tick_timer_service_unsubscribe(void)
{
    SEGGER_RTT_printf(0, "tick_timer_service_unsubscribe START\n\r");
    if (tick_instance != -1)
    {
        event_queue.cancel(tick_instance);
        tick_instance = -1;
    }
    SEGGER_RTT_printf(0, "tick_timer_service_unsubscribe EXIT\n\r");
}

void tick_timer_service_subscribe(Mytime::Windows::TimeUnits time_unit, mbed::Callback<void(struct tm *, Mytime::Windows::TimeUnits)> handler)
{
    SEGGER_RTT_printf(0, "tick_timer_service_subscribe START\n\r");
    // TODO - think need an array to tick_instances so we can 
    if (tick_instance != -1)
    {
        tick_timer_service_unsubscribe();
    }

    tick_instance = event_queue.call_every(time_unit, mbed::callback(&intermediate_ticker), handler, time_unit);
    SEGGER_RTT_printf(0, "tick_timer_service_subscribe EXIT\n\r");
}

#endif /* __WINDOW_API_H__ */
