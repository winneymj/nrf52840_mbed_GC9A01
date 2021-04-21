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

#include <map>
#include <vector>
#include <lvgl/lvgl.h>

extern "C"{
  #include "SEGGER_RTT.h"
}

#define TextLayer lv_obj_t
#define GContext lv_obj_t
#define GFont lv_font_t
#define GColor lv_color_t

extern events::EventQueue app_queue;

typedef struct {
    int16_t x;
    int16_t y;
} GPoint;

typedef struct {
    int16_t w;
    int16_t h;
} GSize;

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
    SEGGER_RTT_printf(0, "wswh E\r\n");                 
    w->setHandlers(handlers);
    SEGGER_RTT_printf(0, "wswh X\r\n");                 
}

Mytime::Windows::Window* window_create()
{
    SEGGER_RTT_printf(0, "wc E\r\n");               

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

    SEGGER_RTT_printf(0, "wc X\r\n");
    return w;
}

void window_destroy(Mytime::Windows::Window* w)
{
    lv_obj_clean(w->getWindow());    
}

typedef struct
{
    int16_t y;
    int16_t max_length;
} Lines;

std::vector<Lines>lines_array;

// std::map<int16_t, int16_t>lines_map;

void initLines(int16_t r, int16_t line_height, std::vector<Lines>& lines)
{
    for (int16_t y = r; y > -r; y -= line_height)
    {
        int16_t h = LV_MATH_ABS(r - y);
// SEGGER_RTT_printf(0, "h=%d\r\n", h);               
        // if (y - line_height < 0)
        // {
        //     h += 20;
        // }

        int16_t length = (int16_t)(2 * sqrt(h * (2 * r - h)));

        if (length && length > 10)
        {
SEGGER_RTT_printf(0, "y=%d length=%d\r\n", y, length);
            lines.push_back({y, length});
        }
    }
}

const std::vector<std::string> explode(const std::string& s, const char& c)
{
	std::string buff{""};
	std::vector<string> v;
	
	for(auto n:s)
	{
		if(n != c) buff+=n; else
		if(n == c && buff != "") { v.push_back(buff); buff = ""; }
	}
	if(buff != "") v.push_back(buff);
	
	return v;
}

typedef struct 
{
    int16_t count;
    int16_t width;
    int16_t text_height;
    std::string text;
} WordData;

void calcAllowableWords(WordData& word_data, lv_obj_t * obj, int16_t maxWidth, std::vector<std::string>& words)
{
    int16_t wordCount = 0;
    std::string testLine = "";
    std::string spacer = "";
    int16_t fittedWidth = 0;
    std::string fittedText = "";
    lv_point_t txt_size = {0};

    // ctx.font = font;

    for (int16_t i = 0; i < words.size(); i++)
    {
        testLine += spacer + words[i];
        spacer = " ";

        // The the width of the text
        lv_style_int_t letter_space = lv_obj_get_style_value_letter_space(obj, LV_LABEL_PART_MAIN);
        lv_style_int_t line_space = lv_obj_get_style_value_letter_space(obj, LV_LABEL_PART_MAIN);
        const lv_font_t * font = lv_obj_get_style_value_font(obj, LV_LABEL_PART_MAIN);

        _lv_txt_get_size(&txt_size, testLine.c_str(), font, letter_space, line_space, LV_COORD_MAX, LV_TXT_FLAG_NONE);
// SEGGER_RTT_printf(0, "txt_size.x=%d,txt_size.y=%d\r\n", txt_size.x, txt_size.y);


        // var width = ctx.measureText(testLine).width;

        if (txt_size.x > maxWidth)
        {
            word_data.count = i;
            word_data.width = fittedWidth;
            word_data.text = fittedText;
            word_data.text_height = txt_size.y;
// SEGGER_RTT_printf(0, "word_data.count=%d,word_data.width=%d,word_data.text=%s\r\n", word_data.count, word_data.width, word_data.text.c_str());
            return;
        }

        fittedWidth = txt_size.x;
        fittedText = testLine;
    }

    word_data.count = words.size();
    word_data.width = txt_size.x;
    word_data.text = testLine;
    word_data.text_height = txt_size.y;
// SEGGER_RTT_printf(0, "word_data.count=%d,word_data.width=%d,word_data.text=%s\r\n", word_data.count, word_data.width, word_data.text.c_str());
}

void wrapText(lv_obj_t * obj, char* text)
{
//   char str[] ="- This, a sample string.";
//   char * pch;
//   SEGGER_RTT_printf(0, "Splitting string \"%s\" into tokens:\n",text);
//   pch = strtok (text," ,.-");
//   while (pch != NULL)
//   {
//     SEGGER_RTT_printf(0, "%s\n",pch);
//     pch = strtok (NULL, " ,.-");
//   }

    std::vector<std::string> words{explode(std::string(text), ' ')};
// 	for(auto n:words)
//     {
// SEGGER_RTT_printf(0, "token=%s\r\n", n.c_str());
//     }

    int16_t i = 0;
   
    // Get page Y position and padding
    lv_area_t page_coords;
    lv_obj_get_coords(obj, &page_coords);
    lv_coord_t bw = lv_obj_get_style_border_width(obj, LV_OBJ_PART_MAIN);
// SEGGER_RTT_printf(0, "bw=%d\r\n", bw);
    lv_style_int_t pad_top = lv_obj_get_style_pad_top(obj, LV_STATE_DEFAULT);
    int16_t wtop = page_coords.y1 + bw + pad_top;

SEGGER_RTT_printf(0, "page_coords.x1=%d,y1=%d,x2=%d,y2=%d\r\n", page_coords.x1, page_coords.y1, page_coords.x2, page_coords.y2);
    // lv_style_int_t pad_inner = lv_obj_get_style_pad_inner(obj, LV_STATE_DEFAULT);
    // lv_style_int_t margin_top = lv_obj_get_style_margin_top(obj, LV_STATE_DEFAULT);
// SEGGER_RTT_printf(0, "pad_top=%d,pad_inner=%d,margin_top=%d\r\n", pad_top, pad_inner, margin_top);

    int16_t pagew = lv_obj_get_width(obj);
    int16_t cx = pagew / 2;
SEGGER_RTT_printf(0, "cx=%d\r\n", cx);

    auto it = lines_array.begin();
    while (it != lines_array.end() && words.size() > 0)
    {
        int16_t line_y = it->y;
        int16_t lines_max_length = it->max_length;
SEGGER_RTT_printf(0, "line_y=%d, lines_max_length=%d\r\n", line_y, lines_max_length);

        WordData line_data;
        calcAllowableWords(line_data, obj, lines_max_length, words);

        static lv_style_t style_label;
        lv_style_init(&style_label);
        // Make the padding/margin really small
        // lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 10);
        // lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 10);
        // lv_style_set_pad_right(&style_label, LV_STATE_DEFAULT, 10);
        // lv_style_set_pad_bottom(&style_label, LV_STATE_DEFAULT, 10);
        // lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 1);

        // lv_style_set_radius(&style_label, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round

        lv_obj_t * label = lv_label_create(obj, NULL);
        // lv_label_set_text(label, "Hello");
        // lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
        lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);

        lv_obj_set_width(label, line_data.width);

        int16_t xpos = cx - (line_data.width / 2);
// SEGGER_RTT_printf(0, "xpos=%d\r\n", xpos);
// SEGGER_RTT_printf(0, "wtop-line_y=%d\r\n", wtop - line_y);
// SEGGER_RTT_printf(0, "wtop-line_y+line_data.text_height=%d\r\n", wtop - line_y + line_data.text_height);
        int16_t ypos = wtop - line_y + line_data.text_height;
SEGGER_RTT_printf(0, "xpos=%d,ypos=%d,wid=%d,text=%s\r\n", xpos, ypos, line_data.width, line_data.text.c_str());
        lv_obj_set_pos(label, xpos, ypos);

// SEGGER_RTT_printf(0, "line_data.count=%d, .width=%d, .text_height=%d, .text=%s\r\n", line_data.count, line_data.width, line_data.text_height, line_data.text.c_str());
// SEGGER_RTT_printf(0, "line_y=%d, ypos=%d\r\n", line_y, ypos);

        lv_label_set_text(label, line_data.text.c_str());
        // lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 1);
        lv_obj_add_style(label, LV_OBJ_PART_MAIN, &style_label);

    //     ctx.fillText(lineData.text, cx - lineData.width / 2, cy - line.y + textHeight);

        // Remove the words already used
        words.erase(words.begin(), words.begin() + line_data.count);
        it++;
    }
}

TextLayer* text_layer_create(Mytime::Windows::Window *parent_window, GRect &rect)
{
    // Create page style
    // static lv_style_t style_page;
    // lv_style_set_bg_color(&style_page, LV_STATE_DEFAULT, LV_COLOR_WHITE);
    // lv_style_set_border_width(&style_page, LV_STATE_DEFAULT, 0);

    // lv_style_copy(&style_page, &style_background);
    // lv_style_set_text_font(&style_page, LV_STATE_DEFAULT, &lv_font_montserrat_18);  /*Set a larger font*/
    // lv_style_set_border_width(&style_page, LV_STATE_DEFAULT, 1);
    // lv_style_set_border_color(&style_page, LV_STATE_DEFAULT, LV_COLOR_BLACK);

    // Create a page
    // lv_obj_t* page = lv_obj_create(parent_window->getWindow(), NULL);
    // lv_obj_add_style(page, LV_OBJ_PART_MAIN, &style_page);
    // lv_obj_set_width(page, rect.width());
    // lv_obj_set_height(page, rect.height());
    // lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);

    /*Create a label on the page*/
    // lv_obj_t* label = lv_label_create(parent_window->getWindow(), NULL);

    // static lv_style_t style_background;
    // lv_style_init(&style_background);
    // lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 1);
    // lv_style_set_radius(&style_background, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round

    // lv_obj_t * label1 = lv_label_create(lv_scr_act(), NULL);
    // // lv_label_set_long_mode(label1, LV_LABEL_LONG_BREAK);     /*Break the long lines*/
    // lv_label_set_recolor(label1, true);                      /*Enable re-coloring by commands in the text*/
    // lv_label_set_align(label1, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    // lv_label_set_text(label1, "Re-color words\nhello\n1\n2\n3\n4\n5");
    // lv_obj_set_width(label1, 150);
    // lv_obj_align(label1, NULL, LV_ALIGN_CENTER, 0, -30);


    // lv_obj_add_style(label1, LV_OBJ_PART_MAIN, &style_background);



    // static lv_style_t style_background;
    // lv_style_init(&style_background);
    // lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 9);
    // lv_style_set_radius(&style_background, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round
    // lv_obj_add_style(label, LV_OBJ_PART_MAIN, &style_background);
    // // lv_obj_add_style(label, LV_OBJ_PART_MAIN, &style_page);
    // lv_obj_set_width(label, 150);
    // lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);       /*Center aligned lines*/
    // lv_obj_set_width(label, rect.width());
    // lv_obj_set_height(label, rect.height());

    static lv_style_t style_background;
    lv_style_init(&style_background);
    lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 2);

    lv_style_set_radius(&style_background, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round

    /*Create a page*/
    lv_obj_t * page = lv_page_create(lv_scr_act(), NULL);
    // lv_obj_set_width(page, 150);
    lv_obj_set_size(page, 150, 150);
    lv_obj_align(page, NULL, LV_ALIGN_CENTER, 0, 0);
    lv_page_set_scrlbar_mode(page, LV_SCRLBAR_MODE_OFF);

    // Make the padding/margin really small
    lv_style_set_pad_left(&style_background, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_top(&style_background, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_right(&style_background, LV_STATE_DEFAULT, 1);
    lv_style_set_pad_bottom(&style_background, LV_STATE_DEFAULT, 1);

    // lv_style_set_margin_top(&style_background, LV_STATE_DEFAULT, 1);

    lv_obj_add_style(page, LV_OBJ_PART_MAIN, &style_background);

    // Now have a round page now figure out labels to fill the round window
    lv_coord_t diam = lv_obj_get_width(page);
    const lv_font_t * font = lv_obj_get_style_value_font(page, LV_LABEL_PART_MAIN);
    initLines(diam / 2, font->line_height, lines_array);
    // wrapText(page, "'Twas the night before Christmas, when all through the house,  Not a creature was stirring, not even a mouse.  And so begins the story of the day of Christmas");
    wrapText(page, "Lorem ipsum dolor sit amet, consectetur adipiscing elit,"
                             "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua."
                             "Ut enim ad minim veniam, quis nostrud exercitation ullamco"
                             "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure"
                             "dolor in reprehenderit in voluptate velit esse cillum dolore"
                             "eu fugiat nulla pariatur."
                             "Excepteur sint occaecat cupidatat non proident, sunt in culpa"
                             "qui officia deserunt mollit anim id est laborum.");




    /*Create a label on the page*/
    // static lv_style_t style_label;
    // lv_style_init(&style_label);
    // // Make the padding/margin really small
    // lv_style_set_pad_left(&style_label, LV_STATE_DEFAULT, 10);
    // lv_style_set_pad_top(&style_label, LV_STATE_DEFAULT, 10);
    // lv_style_set_pad_right(&style_label, LV_STATE_DEFAULT, 10);
    // lv_style_set_pad_bottom(&style_label, LV_STATE_DEFAULT, 10);
    // lv_style_set_border_width(&style_label, LV_STATE_DEFAULT, 1);

    // lv_style_set_radius(&style_label, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round

    // lv_obj_t * label = lv_label_create(page, NULL);
    // // lv_label_set_text(label, "Hello");
    // lv_label_set_long_mode(label, LV_LABEL_LONG_BREAK);            /*Automatically break long lines*/
    // lv_label_set_align(label, LV_LABEL_ALIGN_CENTER);
    // lv_obj_set_width(label, lv_page_get_width_fit(page));          /*Set the label width to max value to not show hor. scroll bars*/

    // lv_label_set_text(label, "Lorem ipsum dolor sit amet");
    // lv_label_set_text(label, "Lorem ipsum dolor sit amet, consectetur adipiscing elit,\n"
    //                          "sed do eiusmod tempor incididunt ut labore et dolore magna aliqua.\n"
    //                          "Ut enim ad minim veniam, quis nostrud exercitation ullamco\n"
    //                          "laboris nisi ut aliquip ex ea commodo consequat. Duis aute irure\n"
    //                          "dolor in reprehenderit in voluptate velit esse cillum dolore\n"
    //                          "eu fugiat nulla pariatur.\n"
    //                          "Excepteur sint occaecat cupidatat non proident, sunt in culpa\n"
    //                          "qui officia deserunt mollit anim id est laborum.");
    // lv_obj_add_style(label, LV_LABEL_PART_MAIN, &style_background);
    // lv_obj_add_style(label, LV_OBJ_PART_MAIN, &style_label);
    // return label;
    return page;
}

void window_stack_push(Mytime::Windows::Window* w, bool animate = false)
{
    SEGGER_RTT_printf(0, "window_stack_push START\r\n");                 
    // Push to the top of the stack and call the load handlers
    windows_array[0] = w;
    Mytime::Windows::WindowHandlers handlers = w->getHandlers();
    if (handlers.load)
    {
        handlers.load(w);
    }
    SEGGER_RTT_printf(0, "window_stack_push EXIT\r\n");                 
}

Mytime::Windows::Window* window_stack_pop(bool animated)
{
    // SEGGER_RTT_printf(0, "stack_pop START\r\n");
    // Pop off and call handlers    
    // Mytime::Windows::Window* w = nullptr;
    Mytime::Windows::Window* w = windows_array[0];
    // SEGGER_RTT_printf(0, "window_stack_pop w=%0x%x\r\n", w);
    Mytime::Windows::WindowHandlers handlers = w->getHandlers();
    // SEGGER_RTT_printf(0, "window_stack_pop handlers=%0x%x\r\n", handlers);
    // SEGGER_RTT_printf(0, "window_stack_pop handlers.unload=%0x%x\r\n", handlers.unload);
    if (handlers.unload)
    {
        handlers.unload();
    }
    // SEGGER_RTT_printf(0, "stack_pop EXIT\r\n");
    return w;
}

void text_layer_set_long_mode(TextLayer *tl, lv_label_long_mode_t mode)
{
    lv_label_set_long_mode(tl, mode);
}

void text_layer_set_size(TextLayer *tl, const GSize max_size)
{
    lv_obj_set_width(tl, max_size.w);
    lv_obj_set_height(tl, max_size.h);
}

void text_layer_set_text_alignment(TextLayer *tl, const lv_label_align_t align)
{
    lv_label_set_align(tl, align);
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

void text_layer_set_alignment(TextLayer *tl, lv_align_t align)
{
    lv_obj_align(tl, NULL, align, 0, 0);
}

// extern events::EventQueue event_queue;

static int tick_instance = -1;

void intermediate_ticker(mbed::Callback<void(struct tm *, Mytime::Windows::TimeUnits)> handler, Mytime::Windows::TimeUnits time_unit)
{
    SEGGER_RTT_printf(0, "it E\r\n");
    time_t seconds = time(NULL);
    struct tm *current_time;
    current_time = localtime(&seconds);

    handler(current_time, time_unit);
    SEGGER_RTT_printf(0, "it X\r\n");
}

void tick_timer_service_unsubscribe(void)
{
    SEGGER_RTT_printf(0, "ttsu E\r\n");
    if (tick_instance != -1)
    {
        app_queue.cancel(tick_instance);
        tick_instance = -1;
    }
    SEGGER_RTT_printf(0, "ttsu X\r\n");
}

void tick_timer_service_subscribe(Mytime::Windows::TimeUnits time_unit, mbed::Callback<void(struct tm *, Mytime::Windows::TimeUnits)> handler)
{
    SEGGER_RTT_printf(0, "ttss E\n\r");
    // TODO - think need an array to tick_instances so we can 
    if (tick_instance != -1)
    {
        tick_timer_service_unsubscribe();
    }

    tick_instance = app_queue.call_every(time_unit, mbed::callback(&intermediate_ticker), handler, time_unit);
    SEGGER_RTT_printf(0, "ttss X\n\r");
}

GFont* fonts_load_custom_font(const char *file_path)
{
    // Note that to load a font LVGL's filesystem needs to be enabled and a driver needs to be added.
    // lv_font_t * my_font;
    // my_font = lv_font_load(file_path);

    return nullptr;
}

void fonts_unload_custom_font(GFont* my_font)
{
    // lv_font_free(my_font);    
}

GContext* graphics_draw_circle(GContext* ctx, GPoint p, uint16_t radius)
{
    lv_obj_t* s_background_obj = lv_obj_create(ctx, NULL);
    lv_obj_set_size(s_background_obj, radius * 2, radius * 2);
    lv_obj_set_pos(s_background_obj, p.x, p.y); // Position off screen

    // Fill base rectangle with TEAL
    static lv_style_t style_background;
    lv_style_init(&style_background);
    lv_style_set_border_width(&style_background, LV_STATE_DEFAULT, 9);
    lv_style_set_radius(&style_background, LV_STATE_DEFAULT, LV_RADIUS_CIRCLE); // Make round
    lv_obj_add_style(s_background_obj, LV_OBJ_PART_MAIN, &style_background);

    return s_background_obj;
}

void window_set_background_color(GContext* ctx, GColor background_color)
{
    static lv_style_t style_background;
    lv_style_init(&style_background);
    lv_style_set_bg_color(&style_background, LV_STATE_DEFAULT, background_color);
    lv_obj_add_style(ctx, LV_OBJ_PART_MAIN, &style_background);
}
#endif /* __WINDOW_API_H__ */
