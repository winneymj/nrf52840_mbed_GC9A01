/* mbed Microcontroller Library
 * Copyright (c) 2006-2015 ARM Limited
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
#ifndef _PERIPHERAL_H_
#define _PERIPHERAL_H_

#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/services/HeartRateService.h"
 
const static char DEVICE_NAME[] = "Heartrate";
 
class Peripheral : ble::Gap::EventHandler {
public:
    Peripheral(BLE &ble, events::EventQueue &event_queue) :
        _ble(ble),
        _event_queue(event_queue),
        _connected(false),
        _hr_uuid(GattService::UUID_HEART_RATE_SERVICE),
        _hr_counter(100),
        _hr_service(ble, _hr_counter, HeartRateService::LOCATION_FINGER),
        _adv_data_builder(_adv_buffer) { }
 
    void start();
 
private:
    /** Callback triggered when the ble initialization process has finished */
    void on_init_complete(BLE::InitializationCompleteCallbackContext *params);
    void start_advertising();
     void update_sensor_value();
 
private:
    /* Event handler */
 
    void onDisconnectionComplete(const ble::DisconnectionCompleteEvent&);
    virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event);
 
private:
    BLE &_ble;
    events::EventQueue &_event_queue;
 
    bool _connected;
 
    UUID _hr_uuid;
 
    uint8_t _hr_counter;
    HeartRateService _hr_service;
 
    uint8_t _adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    ble::AdvertisingDataBuilder _adv_data_builder;
};

#endif
