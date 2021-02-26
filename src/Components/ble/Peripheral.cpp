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
 
#include <events/mbed_events.h>
#include <mbed.h>
#include "ble/BLE.h"
#include "ble/gap/Gap.h"
#include "ble/services/HeartRateService.h"
#include "pretty_printer.h"
#include "Peripheral.h"
extern "C"{
  #include "SEGGER_RTT.h"
}
 
void Peripheral::start()
{
    SEGGER_RTT_printf(0, "start:ENTER\n");
    _ble.gap().setEventHandler(this);

    _ble.init(this, &Peripheral::on_init_complete);

    _event_queue.call_every(1000, this, &Peripheral::update_sensor_value);

    SEGGER_RTT_printf(0, "start:before dispatch_forever\n");
    _event_queue.dispatch_forever();
}
 
/** Callback triggered when the ble initialization process has finished */
void Peripheral::on_init_complete(BLE::InitializationCompleteCallbackContext *params)
{
    SEGGER_RTT_printf(0, "start:on_init_complete\n");
    if (params->error != BLE_ERROR_NONE) {
        SEGGER_RTT_printf(0, "Ble initialization failed.");
        return;
    }

    print_mac_address();

    start_advertising();
}
 
void Peripheral::start_advertising()
{
    SEGGER_RTT_printf(0, "start_advertising:ENTER\n");
    /* Create advertising parameters and payload */

    ble::AdvertisingParameters adv_parameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED,
        ble::adv_interval_t(ble::millisecond_t(1000))
    );

    _adv_data_builder.setFlags();
    _adv_data_builder.setAppearance(ble::adv_data_appearance_t::GENERIC_HEART_RATE_SENSOR);
    _adv_data_builder.setLocalServiceList(mbed::make_Span(&_hr_uuid, 1));
    _adv_data_builder.setName(DEVICE_NAME);

    /* Setup advertising */

    ble_error_t error = _ble.gap().setAdvertisingParameters(
        ble::LEGACY_ADVERTISING_HANDLE,
        adv_parameters
    );

    if (error) {
        SEGGER_RTT_printf(0, "_ble.gap().setAdvertisingParameters() failed\r\n");
        return;
    }

    error = _ble.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE,
        _adv_data_builder.getAdvertisingData()
    );

    if (error) {
        SEGGER_RTT_printf(0, "_ble.gap().setAdvertisingPayload() failed\r\n");
        return;
    }

    /* Start advertising */

    error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);

    if (error) {
        SEGGER_RTT_printf(0, "_ble.gap().startAdvertising() failed\r\n");
        return;
    }

    SEGGER_RTT_printf(0, "start_advertising:EXIT\n");
}
 
void Peripheral::update_sensor_value()
{
    SEGGER_RTT_printf(0, "update_sensor_value:ENTER\n");
    if (_connected) {
        // Do blocking calls or whatever is necessary for sensor polling.
        // In our case, we simply update the HRM measurement.
        _hr_counter++;

        //  100 <= HRM bps <=175
        if (_hr_counter == 175) {
            _hr_counter = 100;
        }

        _hr_service.updateHeartRate(_hr_counter);
    }
}
 
void Peripheral::onDisconnectionComplete(const ble::DisconnectionCompleteEvent&)
{
    SEGGER_RTT_printf(0, "onDisconnectionComplete:ENTER\n");
    _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    _connected = false;
}
 
void Peripheral::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    SEGGER_RTT_printf(0, "onConnectionComplete:ENTER\n");
    if (event.getStatus() == BLE_ERROR_NONE) {
        _connected = true;
    }
}
 
