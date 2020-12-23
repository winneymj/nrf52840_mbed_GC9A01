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

#include "mbed.h"
#include "bma423_main.h"
#include "bma423.h"
#include "bma4_defs.h"
#include "bma4_common.h"

struct bma4_dev _bma;

void BMA423_motion_interrupt()
{
  struct bma4_accel_config accel_conf;
  struct bma423_any_no_mot_config any_no_mot;
  uint16_t int_status = 0;
  /* Loop variable */
  uint8_t iteration = 20;

  /* Accelerometer Configuration Setting */
  /* Output data Rate */
  accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;

  /* Gravity range of the sensor (+/- 2G, 4G, 8G, 16G) */
  accel_conf.range = BMA4_ACCEL_RANGE_2G;

  /* Bandwidth configure number of sensor samples required to average
    * if value = 2, then 4 samples are averaged
    * averaged samples = 2^(val(accel bandwidth))
    * Note1 : More info refer datasheets
    * Note2 : A higher number of averaged samples will result in a lower noise level of the signal, but
    * since the performance power mode phase is increased, the power consumption will also rise.
    */
  accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /* Enable the filter performance mode where averaging of samples
    * will be done based on above set bandwidth and ODR.
    * There are two modes
    *  0 -> Averaging samples (Default)
    *  1 -> No averaging
    * For more info on No Averaging mode refer datasheets.
    */
  accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

  /* Set the accel configurations */
  int8_t rslt = bma4_set_accel_config(&accel_conf, &_bma);
  bma4_error_codes_print_result("bma4_set_accel_config status", rslt);

  /* Select the axis for which any/no motion interrupt should be generated */
  any_no_mot.axes_en = BMA423_EN_ALL_AXIS;

  /*
    * Set the slope threshold:
    * Interrupt will be generated if the slope of all the axis exceeds the threshold (1 bit = 0.48mG)
    */
  any_no_mot.threshold = 10;

  /*
    * Set the duration for any/no motion interrupt:
    * Duration defines the number of consecutive data points for which threshold condition must be true(1 bit = 20ms)
    */
  any_no_mot.duration = 4;

  /* Set the threshold, duration and axis enable configuration */
  rslt = bma423_set_any_mot_config(&any_no_mot, &_bma);
  bma4_error_codes_print_result("bma423_set_any_mot_config status", rslt);
  // rslt = bma423_set_no_mot_config(&any_no_mot, &_bma);
  // bma4_error_codes_print_result("bma423_set_no_mot_config status", rslt);


  struct bma4_int_pin_config pin_config;
  pin_config.output_en = BMA4_OUTPUT_ENABLE;
  pin_config.lvl = BMA4_ACTIVE_LOW;
  pin_config.edge_ctrl = BMA4_LEVEL_TRIGGER;
  pin_config.od = BMA4_PUSH_PULL;

  rslt = bma4_set_int_pin_config(&pin_config, BMA4_INTR1_MAP, &_bma);
  bma4_error_codes_print_result("bma4_set_int_pin_config status", rslt);

  /* Set interrupt mode so it does not stay latched. */
  rslt = bma4_set_interrupt_mode(BMA4_NON_LATCH_MODE, &_bma);
  bma4_error_codes_print_result("bma4_set_interrupt_mode status", rslt);

  /* Map the interrupt pin with that of any-motion and no-motion interrupts.
    * Interrupt will be generated when any or no-motion is recognized.
    */
  rslt = bma423_map_interrupt(BMA4_INTR1_MAP, BMA423_ANY_MOT_INT /*| BMA423_NO_MOT_INT*/, BMA4_ENABLE, &_bma);
  bma4_error_codes_print_result("bma423_map_interrupt status", rslt);

  printf("Shake the board for any-motion interrupt whereas do not shake the board for no-motion interrupt\n");

  DigitalIn int1_pin(P0_11);

  while (1)
  {
    // Read actual pins
    int val = int1_pin.read();
    if (val == 0)
    {
      printf("int1_pin=%d\r\n", val);
    }

    // /* Read the interrupt register to check whether any-motion or no-motion interrupt is received */
    rslt = bma423_read_int_status(&int_status, &_bma);

    /* Check if any-motion interrupt is triggered */
    if (int_status & BMA423_ANY_MOT_INT)
    {
        printf("Any-Motion interrupt received\r\n");
        iteration--;
    }

    // // /* Check if no-motion interrupt is triggered */
    // // if (int_status & BMA423_NO_MOT_INT)
    // // {
    // //     printf("No-Motion interrupt received\n");
    // //     iteration--;
    // // }

    // int_status = 0;

    // /* Break out of the loop when iteration has reached zero */
    // if (iteration == 0)
    // {
    //     printf("Iterations are done. Exiting !");
    //     break;
    // }
  }
}

void BMA423_wakeup_interrupt()
{
  // TODO See if we can get wakeup tap interrupts working

    /* Loop variable */
  uint8_t iteration = 10;

  /* Define a variable to get the status */
  uint16_t int_status = 0;
  uint8_t int_line[2] = { BMA4_INTR1_MAP, BMA4_INTR2_MAP };

  /* Variable to define feature interrupts to be mapped */
  uint16_t int_map = (BMA423_SINGLE_TAP_INT | BMA423_DOUBLE_TAP_INT);

  struct bma4_accel_config accel_conf;
  /* Accelerometer Configuration Setting */
  /* Output data Rate */
  accel_conf.odr = BMA4_OUTPUT_DATA_RATE_100HZ;

  /* Gravity range of the sensor (+/- 2G, 4G, 8G, 16G) */
  accel_conf.range = BMA4_ACCEL_RANGE_2G;

  /* Bandwidth configure number of sensor samples required to average
    * if value = 2, then 4 samples are averaged
    * averaged samples = 2^(val(accel bandwidth))
    * Note1 : More info refer datasheets
    * Note2 : A higher number of averaged samples will result in a lower noise level of the signal, but since the
    * performance power mode phase is increased, the power consumption will also rise.
    */
  accel_conf.bandwidth = BMA4_ACCEL_NORMAL_AVG4;

  /* Enable the filter performance mode where averaging of samples
    * will be done based on above set bandwidth and ODR.
    * There are two modes
    *  0 -> Averaging samples (Default)
    *  1 -> No averaging
    * For more info on No Averaging mode refer datasheets.
    */
  accel_conf.perf_mode = BMA4_CIC_AVG_MODE;

  /* Set the accel configurations */
  int8_t rslt = bma4_set_accel_config(&accel_conf, &_bma);
  bma4_error_codes_print_result("bma4_set_accel_config status", rslt);

  /* Enable single & double tap feature */
  rslt = bma423_feature_enable(BMA423_SINGLE_TAP | BMA423_DOUBLE_TAP, 1, &_bma);
  bma4_error_codes_print_result("bma423_feature_enable status", rslt);

  /* Mapping line interrupt 1 with that of two sensor feature interrupts -
    * double tap and single tap interrupt */
  rslt = bma423_map_interrupt(int_line[0], int_map, BMA4_ENABLE, &_bma);
  bma4_error_codes_print_result("bma423_map_interrupt", rslt);

  if (BMA4_OK == rslt)
  {
    printf("Do Single or Double Tap the board\n");

    while (1)
    {
      /* Read the interrupt register to check whether single tap or double tap interrupt is received */
      rslt = bma423_read_int_status(&int_status, &_bma);
      if (int_status & BMA423_SINGLE_TAP_INT)
      {
        printf("Single tap received\n");
        iteration--;
      }
      else if (int_status & BMA423_DOUBLE_TAP_INT)
      {
        printf("Double tap received\n");
        iteration--;
      }

      int_status = 0;

      /* Break out of the loop when iteration has reached zero */
      if (iteration == 0)
      {
        printf("Iterations are done. Exiting !");
        break;
      }
    }
  }
}

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

  // BMA423_motion_interrupt();

  return rslt;
}
