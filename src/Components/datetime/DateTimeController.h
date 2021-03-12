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

#ifndef __DATE_TIME_CONTROLLER_H__
#define __DATE_TIME_CONTROLLER_H__

#include "mbed.h"
#include "mbed_mktime.h"

namespace Mytime {
  namespace Controllers {
    class DateTimeController {
      public:
        enum class Days : uint8_t {Unknown, Monday, Tuesday, Wednesday, Thursday, Friday, Saturday, Sunday};
        enum class Months : uint8_t {Unknown, January, February, March, April, May, June, July, August, September, October, November, December};

        void SetTime(uint16_t year, uint8_t month, uint8_t day, uint8_t dayOfWeek, uint8_t hour, uint8_t minute, uint8_t second);
        // uint16_t Year() const { return year; }
        // Months Month() const { return month; }
        // uint8_t Day() const { return day; }
        // Days DayOfWeek() const { return dayOfWeek; }
        // uint8_t Hours() const { return hour; }
        // uint8_t Minutes() const { return minute; }
        // uint8_t Seconds() const { return second; }

        time_t CurrentDateTime();

      private:
        // uint16_t year = 0;
        // Months month = Months::Unknown;
        // uint8_t day = 0;
        // Days dayOfWeek = Days::Unknown;
        // uint8_t hour = 0;
        // uint8_t minute = 0;
        // uint8_t second = 0;

        time_t currentDateTime;
    };
  }
}

#endif //__DATE_TIME_CONTROLLER_H__