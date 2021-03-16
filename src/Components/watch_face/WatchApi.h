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


static Mytime::Windows::Window* s_main_window;
static TextLayer *s_time_layer;

static void main_window_load(Mytime::Windows::Window* w)
{
    SEGGER_RTT_printf(0, "main_window_load START\n\r");

    // Create the TextLayer with specific bounds
    GRect bounds = GRect(0, 100, 249, 175);
    s_time_layer = text_layer_create(w, bounds);

    // Improve the layout to be more like a watchface
    text_layer_set_background_color(s_time_layer, LV_COLOR_BLUE);
    text_layer_set_text_color(s_time_layer, LV_COLOR_BLACK);
    text_layer_set_text(s_time_layer, "00:00");
    text_layer_set_font(s_time_layer, &lv_font_montserrat_18);
    text_layer_set_text_alignment(s_time_layer, LV_ALIGN_CENTER);

    SEGGER_RTT_printf(0, "main_window_load EXIT\n\r");
}

static void main_window_unload(/*Window *window*/)
{
    SEGGER_RTT_printf(0, "main_window_unload START\n\r");
    SEGGER_RTT_printf(0, "main_window_unload EXIT\n\r");
}

namespace Mytime {
    namespace Controllers {
        /**
         */
        class WatchAPI
        {
        public:
            ~WatchAPI() {};
            WatchAPI(events::EventQueue& event_queue): _event_queue(event_queue) {};

            void init()
            {
                SEGGER_RTT_printf(0, "WatchAPI:init START\n\r");

                s_main_window = window_create();

                window_set_window_handlers(s_main_window, (Mytime::Windows::WindowHandlers)
                {
                    .load = mbed::callback(&main_window_load),
                    .unload = mbed::callback(&main_window_unload)
                });

                window_stack_push(s_main_window);

                SEGGER_RTT_printf(0, "WatchAPI:init EXIT\n\r");
            };

            void deinit()
            {
                SEGGER_RTT_printf(0, "WatchAPI:init START\n\r");
                window_destroy(s_main_window);
                SEGGER_RTT_printf(0, "WatchAPI:init EXIT\n\r");
            };

            void main()
            {
                SEGGER_RTT_printf(0, "WatchAPI:main START\n\r");                 

                init();
                _event_queue.dispatch_forever();
                deinit();

                SEGGER_RTT_printf(0, "WatchAPI:main EXIT\n\r");                 
            };

        private:
            events::EventQueue& _event_queue;
        };
    }
}

#endif /* __WATCH_API_H__ */
