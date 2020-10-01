/* mbed Microcontroller Library
 * Copyright (c) 2019 ARM Limited
 * SPDX-License-Identifier: Apache-2.0
 */
#ifdef MBED_BLINKY_EXAMPLE
#ifndef MBED_TEST_MODE
#include "mbed.h"
// Comment in the USBConsole.h include below to send printf output back through USB to computer.
// Open serial terminal session on COM port using terminal program such as Putty to see output 
//#include "USBConsole.h"  
 
// Blinking rate in milliseconds
#define BLINKING_RATE_MS     500
 
int main()
{
    // Initialise the digital pin LED1 as an output
    DigitalOut led(LED1);
 
    while (true) {
        led = !led;           
        printf("Blinky\r\n");
        ThisThread::sleep_for(BLINKING_RATE_MS);
    }
}
#endif /* MBED_TEST_MODE */
#endif /* MBED_BLINKY_EXAMPLE */