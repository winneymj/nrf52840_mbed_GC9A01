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
#include "Components/ble/AlertNotificationService.h"
#include "Components/ble/NotificationManager.h"
#include "Components/datetime/DateTimeController.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

using namespace Mytime::Controllers;

int main() {
    SEGGER_RTT_printf(0, "main: START\r\n");

    BLE &ble_interface = BLE::Instance();
    events::EventQueue event_queue;
    DateTime date_time_controller;
    NotificationManager notification_manager;

    CurrentTimeService current_time_service(date_time_controller);
    AlertNotificationService alert_notification_service(notification_manager);
    BLEProcess ble_process(event_queue, ble_interface);

    mbed::Callback<void(BLE&, events::EventQueue&)> post_init_cb[] = {
        callback(&current_time_service, &CurrentTimeService::start),
        callback(&alert_notification_service, &AlertNotificationService::start),
        NULL
    };

    SEGGER_RTT_printf(0, "main: ble_process.on_init()\r\n");
    ble_process.on_init(post_init_cb);

    // bind the event queue to the ble interface, initialize the interface
    // and start advertising
    SEGGER_RTT_printf(0, "main: ble_process.start()\r\n");
    ble_process.start();

    return 0;
}

