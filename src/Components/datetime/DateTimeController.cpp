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

#include "mbed.h"
#include "mbed_mktime.h"

#include "DateTimeController.h"
extern "C"{
  #include "SEGGER_RTT.h"
}
// #include <date/date.h>
// #include <libraries/log/nrf_log.h>

using namespace Mytime::Controllers;


void DateTimeController::SetTime(uint16_t year, uint8_t month, uint8_t day,
                        uint8_t dayOfWeek, uint8_t hour, uint8_t minute,
                        uint8_t second) {
  struct tm tm = {
    /*	tm_sec */  second,
    /*	tm_min */  minute,
    /*	tm_hour */ hour,
    /*	tm_mday */ day,
    /*	tm_mon */  month - 1,
    /*	tm_year */ year - 1900
  };

  time_t conv_result;
  bool res = _rtc_maketime(&tm, &conv_result, RTC_4_YEAR_LEAP_YEAR_SUPPORT);
  SEGGER_RTT_printf(0, "_rtc_maketime = %d\n", res);

  if (res)
  {
    set_time(conv_result);
  }
  
  SEGGER_RTT_printf(0, "%d-%d-%d \n", day, month, year);
  SEGGER_RTT_printf(0, "%d:%d:%d \n ", hour, minute, second);
}

time_t DateTimeController::CurrentDateTime()
{
  return time(NULL);
}
