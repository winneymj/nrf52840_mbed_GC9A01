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

#ifndef __WATCH_API_H__
#define __WATCH_API_H__

#include "mbed.h"
#include "Api.h"
#include "Window.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

events::EventQueue app_queue;

static Mytime::Windows::Window* s_main_window;
static TextLayer *s_time_layer;

static void main_window_load(Mytime::Windows::Window* w)
{
    SEGGER_RTT_printf(0, "mwl E\n\r");

    // Create the TextLayer with specific bounds
    GRect bounds = GRect(0, 100, 249, 175);
    s_time_layer = text_layer_create(w, bounds);

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, LV_COLOR_BLUE);
    text_layer_set_text_color(s_time_layer, LV_COLOR_BLACK);
    text_layer_set_text(s_time_layer, "00:00");

    text_layer_set_font(s_time_layer, &lv_font_montserrat_36);
    text_layer_set_text_alignment(s_time_layer, LV_ALIGN_CENTER);

    SEGGER_RTT_printf(0, "mwl X\n\r");
}

static void main_window_unload(/*Window *window*/)
{
    SEGGER_RTT_printf(0, "mwu E\n\r");
    // Unsubscribe from timer/Ticker service
    tick_timer_service_unsubscribe();

    SEGGER_RTT_printf(0, "mwu X\n\r");
}

static void update_time()
{
    SEGGER_RTT_printf(0, "ut E\r\n");
    // Get a tm structure
    time_t temp = time(NULL);
    struct tm *tick_time = localtime(&temp);

    // Write the current hours and minutes into a buffer
    static char s_buffer[8];
    strftime(s_buffer, sizeof(s_buffer), "%H:%M", tick_time);

    SEGGER_RTT_printf(0, "update_time s_buffer=%s\r\n", s_buffer);

    // Display this time on the TextLayer
    text_layer_set_text(s_time_layer, s_buffer);

    SEGGER_RTT_printf(0, "ut X\r\n");
}

static void tick_handler(struct tm *tick_time, Mytime::Windows::TimeUnits units_changed)
{
    SEGGER_RTT_printf(0, "th E\r\n", units_changed);
    update_time();
    SEGGER_RTT_printf(0, "th X\r\n");
}

namespace Mytime {
    namespace Controllers {
        /**
         */
        class WatchAPI
        {
        public:
            ~WatchAPI() {};
            WatchAPI(/*events::EventQueue& event_queue*/) {};

            void init()
            {
                SEGGER_RTT_printf(0, "wi E\r\n");

                s_main_window = window_create();

                window_set_window_handlers(s_main_window, (Mytime::Windows::WindowHandlers)
                {
                    .load = mbed::callback(&main_window_load),
                    .unload = mbed::callback(&main_window_unload)
                });

                window_stack_push(s_main_window);

                // Register with TickTimerService
                tick_timer_service_subscribe(Mytime::Windows::MINUTE_UNIT, &tick_handler);

                SEGGER_RTT_printf(0, "wi X\r\n");
            };

            void deinit()
            {
                SEGGER_RTT_printf(0, "wdi E\r\n");
                window_stack_pop(false);
                window_destroy(s_main_window);
                SEGGER_RTT_printf(0, "wdi X\r\n");
            };

            void main()
            {
                SEGGER_RTT_printf(0, "wm E\r\n");                 

                init();
                app_queue.dispatch_forever();
                deinit();

                SEGGER_RTT_printf(0, "wm X\r\n");                 
            };

        // private:
            // events::EventQueue& _event_queue;
        };
    }
}

#endif /* __WATCH_API_H__ */
