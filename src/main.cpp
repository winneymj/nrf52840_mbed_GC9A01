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

#include <stdio.h>

#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "platform/NonCopyable.h"

#include "ble/GattServer.h"
#include "BLEProcess.h"
#include "Components/ble/CurrentTimeService.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

using namespace Mytime::Controllers;

int main() {
    BLE &ble_interface = BLE::Instance();
    events::EventQueue event_queue;
    DateTime date_time_controller;
    CurrentTimeService demo_service(date_time_controller);
    BLEProcess ble_process(event_queue, ble_interface);

    ble_process.on_init(callback(&demo_service, &CurrentTimeService::start));

    // bind the event queue to the ble interface, initialize the interface
    // and start advertising
    ble_process.start();

    return 0;
}

