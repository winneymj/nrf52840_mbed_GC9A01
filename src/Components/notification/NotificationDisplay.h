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

#ifndef __NOTIFICATION_DISPLAY_H__
#define __NOTIFICATION_DISPLAY_H__

#include "mbed.h"
#include "Api.h"
#include "Window.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

extern events::EventQueue app_queue;

static Mytime::Windows::Window* not_main_window;
static TextLayer *not_text_layer;

static void notif_main_window_load(Mytime::Windows::Window* w)
{
    // SEGGER_RTT_printf(0, "mwl E\n\r");

    // Should be animation but cannot do that with SVG library right now
    // perhaps in the future.

    // GContext* circle = graphics_draw_circle(w->getWindow(), GPoint{.x=0, .y=-150}, 120);
    // GContext* circle = graphics_draw_circle(w->getWindow(), GPoint{.x=50, .y=50}, 75);
    // window_set_background_color(circle, LV_COLOR_TEAL);

    // Create the TextLayer with specific bounds
    GRect bounds = GRect(0, 120, 200, 190);
    not_text_layer = text_layer_create(w, bounds);
    
    // text_layer_set_long_mode(not_text_layer, LV_LABEL_LONG_BREAK);  // ** Set this before the text to make work
    // text_layer_set_size(not_text_layer, GSize{.w = 200, .h = 75}); // ** Set this after the long mode
    // text_layer_set_background_color(not_text_layer, LV_COLOR_WHITE);
    // text_layer_set_text_color(not_text_layer, LV_COLOR_BLACK);
    // text_layer_set_text_alignment(not_text_layer, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    // text_layer_set_font(not_text_layer, &lv_font_montserrat_14);
    // text_layer_set_text(not_text_layer, "this is a longer message than expected so I wonder what happens");

    // SEGGER_RTT_printf(0, "mwl X\n\r");
}

static void notif_main_window_unload(/*Window *window*/)
{
    // SEGGER_RTT_printf(0, "mwu E\r\n");
    // SEGGER_RTT_printf(0, "mwu X\r\n");
}

namespace Mytime {
    namespace Controllers {
        /**
         */
        class NotificationDisplay
        {
        public:
            ~NotificationDisplay() {};
            NotificationDisplay(/*events::EventQueue& event_queue*/) {};

            void init()
            {
                // SEGGER_RTT_printf(0, "wi E\r\n");

                not_main_window = window_create();

                window_set_window_handlers(not_main_window, (Mytime::Windows::WindowHandlers)
                {
                    .load = mbed::callback(&notif_main_window_load),
                    .unload = mbed::callback(&notif_main_window_unload)
                });

                window_stack_push(not_main_window);

                // SEGGER_RTT_printf(0, "wi X\r\n");
            };

            void deinit()
            {
                // SEGGER_RTT_printf(0, "wdi E\r\n");
                window_stack_pop(false);
                window_destroy(not_main_window);
                // SEGGER_RTT_printf(0, "wdi X\r\n");
            };

            void main()
            {
                // SEGGER_RTT_printf(0, "-----------------------wm E\r\n");                 

                init();
                app_queue.dispatch_forever();
                deinit();

                // SEGGER_RTT_printf(0, "-----------------------wm X\r\n");                 
            };
        };
    }
}

#endif /* __WATCH_API_H__ */
