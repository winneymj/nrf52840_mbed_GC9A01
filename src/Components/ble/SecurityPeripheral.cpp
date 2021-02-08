/* mbed Microcontroller Library
 * Copyright (c) 2006-2013 ARM Limited
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
#include "ble/BLE.h"
#include "SecurityPeripheral.h"
#include "SecurityMgr.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

using namespace MiTime::Components;

extern const char DEVICE_NAME[];

/** Set up and start advertising accepting anyone */
void SecurityPeripheral::start()
{
    SEGGER_RTT_printf(0, "SecurityPeripheral::start(): ENTER\r\n");

    ble::peripheral_privacy_configuration_t privacy_configuration = {
        /* use_non_resolvable_random_address */ false,
        ble::peripheral_privacy_configuration_t::PERFORM_PAIRING_PROCEDURE
    };

    if (_bonded) {
        /** advertise and filter based on known devices */
        SEGGER_RTT_printf(0, "We are bonded, we will only accept known devices\r\n");
        privacy_configuration.resolution_strategy =
            ble::peripheral_privacy_configuration_t::REJECT_NON_RESOLVED_ADDRESS;
    }

    _ble.gap().setPeripheralPrivacyConfiguration(&privacy_configuration);

    start_advertising();

    SEGGER_RTT_printf(0, "SecurityPeripheral::start(): EXIT\r\n");
};

void SecurityPeripheral::start_advertising()
{
    SEGGER_RTT_printf(0, "start_advertising(): ENTER\r\n");

    uint8_t adv_buffer[ble::LEGACY_ADVERTISING_MAX_SIZE];
    /* use the helper to build the payload */
    ble::AdvertisingDataBuilder adv_data_builder(adv_buffer);

    SEGGER_RTT_printf(0, "start_advertising(): %s\r\n", DEVICE_NAME);

    adv_data_builder.setFlags();
    adv_data_builder.setName(DEVICE_NAME);

    /* Set payload for the set */
    ble_error_t error = _ble.gap().setAdvertisingPayload(
        ble::LEGACY_ADVERTISING_HANDLE,
        adv_data_builder.getAdvertisingData()
    );

    if (error) {
        SEGGER_RTT_printf(0, "Gap::setAdvertisingPayload() failed:error=%d", error);
        return;
    }

    ble::AdvertisingParameters adv_parameters(
        ble::advertising_type_t::CONNECTABLE_UNDIRECTED
    );

    error = _ble.gap().setAdvertisingParameters(ble::LEGACY_ADVERTISING_HANDLE, adv_parameters);

    if (error) {
        SEGGER_RTT_printf(0, "Gap::setAdvertisingParameters() failed:error=%d", error);
        return;
    }

    if (_bonded) {
        /* if we bonded it means we have found the other device, from now on
            * wait at each step until completion */
        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE);
    } else {
        /* since we have two boards which might start running this example at the same time
            * we randomise the interval of advertising to have them meet when one is advertising
            * and the other one is scanning (we use their random address as source of randomness) */
        ble::millisecond_t random_duration_ms((5 + rand() % 6) * 1000);
        ble::adv_duration_t random_duration(random_duration_ms);

        error = _ble.gap().startAdvertising(ble::LEGACY_ADVERTISING_HANDLE, random_duration);
    }

    if (error) {
        SEGGER_RTT_printf(0, "Gap::startAdvertising() failed:error=%d", error);
        return;
    }

    SEGGER_RTT_printf(0, "Advertising...\r\n");
}
