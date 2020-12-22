/**  @file bma42x I2C device class file */
/* 
================================================================================
I2Cdev device library code is placed under the MIT license
Copyright (c) 2020 Mark Winney (mbed port)

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
================================================================================
*/
#include "bma423_main.h"
#include "bma423.h"
#include "bma4_defs.h"
#include "bma4_common.h"

struct bma4_dev _bma;

/** @brief High level Init, most settings remain at Power-On reset value
 */
int8_t BMA423_init() {

  // I2C or SPI
  int8_t rslt = bma4_interface_selection(&_bma, BMA42X_B_VARIANT);
  bma4_error_codes_print_result("bma4_interface_selection", rslt);

  // Sensor initialization
  rslt = bma423_init(&_bma);
  bma4_error_codes_print_result("bma423_init", rslt);

  // Upload Configuration file to enable the features of the sensor
  rslt = bma423_write_config_file(&_bma);
  bma4_error_codes_print_result("bma423_write_config_file", rslt);

  // Enable the accelerometer
  rslt = bma4_set_accel_enable(1, &_bma);
  bma4_error_codes_print_result("bma4_set_accel_enable", rslt);

  // TODO See if we can get interrupts working


  return rslt;
}
