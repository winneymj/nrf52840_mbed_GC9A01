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
#include "SecurityCentral.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

extern const char DEVICE_NAME[];

using namespace MiTime::Components;

/** start scanning and attach a callback that will handle advertisements
 *  and scan requests responses */
void SecurityCentral::start()
{
    ble::central_privacy_configuration_t privacy_configuration = {
        /* use_non_resolvable_random_address */ false,
        ble::central_privacy_configuration_t::DO_NOT_RESOLVE
    };
    if (_bonded) {
        SEGGER_RTT_printf(0, "We are bonded - we will only see known devices\r\n");
        privacy_configuration.resolution_strategy = ble::central_privacy_configuration_t::RESOLVE_AND_FILTER;
    }

    _ble.gap().setCentralPrivacyConfiguration(&privacy_configuration);

    start_scanning();
}

bool SecurityCentral::start_scanning()
{
    ble_error_t error;
    ble::ScanParameters scan_params;
    _ble.gap().setScanParameters(scan_params);

    _is_connecting = false;

    if (_bonded) {
        /* if we bonded it means we have found the other device, from now on
            * wait at each step until completion */
        error = _ble.gap().startScan(ble::scan_duration_t::forever());
    } else {
        /* otherwise only scan for a limited time before changing roles again
            * if we fail to find the other device */
        error = _ble.gap().startScan(ble::scan_duration_t(ble::millisecond_t(5000)));
    }

    if (error) {
        SEGGER_RTT_printf(0, "Error during Gap::startScan:error %d\r\n", error);
        return false;
    }

    SEGGER_RTT_printf(0, "Scanning...\r\n");

    return true;
}

/** Look at scan payload to find a peer device and connect to it */
void SecurityCentral::onAdvertisingReport(const ble::AdvertisingReportEvent &event)
{
    /* don't bother with analysing scan result if we're already connecting */
    if (_is_connecting) {
        return;
    }

    ble::AdvertisingDataParser adv_data(event.getPayload());

    /* parse the advertising payload, looking for a discoverable device */
    while (adv_data.hasNext()) {
        ble::AdvertisingDataParser::element_t field = adv_data.next();

        /* connect to a known device by name */
        if (field.type == ble::adv_data_type_t::COMPLETE_LOCAL_NAME &&
            field.value.size() == strlen(DEVICE_NAME) &&
            (memcmp(field.value.data(), DEVICE_NAME, field.value.size()) == 0)) {

            SEGGER_RTT_printf(0, "We found a connectable device: \r\n");
            // print_address(event.getPeerAddress().data());

            ble_error_t error = _ble.gap().stopScan();

            if (error) {
                SEGGER_RTT_printf(0, "Error caused by Gap::stopScan:error=%d\n", error);
                return;
            }

            error = _ble.gap().connect(
                event.getPeerAddressType(),
                event.getPeerAddress(),
                ble::ConnectionParameters()
            );

            SEGGER_RTT_printf(0, "Connecting...\r\n");

            if (error) {
                SEGGER_RTT_printf(0, "Error caused by Gap::connect:error=%d\n", error);
                return;
            }

            /* we may have already scan events waiting
                * to be processed so we need to remember
                * that we are already connecting and ignore them */
            _is_connecting = true;

            return;
        }
    }
}
