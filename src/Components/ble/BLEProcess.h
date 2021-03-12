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

#ifndef GATT_SERVER_EXAMPLE_BLE_PROCESS_H_
#define GATT_SERVER_EXAMPLE_BLE_PROCESS_H_

#include <stdint.h>
#include "pretty_printer.h"

#include <events/mbed_events.h>
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "gap/AdvertisingDataParser.h"
#include "ble/common/FunctionPointerWithContext.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

static const char DEVICE_NAME[] = "InfiniTime";
static const uint16_t MAX_ADVERTISING_PAYLOAD_SIZE = 50;

namespace Mytime {
    namespace Controllers {
        /**
         * Handle initialization and shutdown of the BLE Instance.
         *
         * Setup advertising payload and manage advertising state.
         * Delegate to GattClientProcess once the connection is established.
         */
        class BLEProcess : private mbed::NonCopyable<BLEProcess>, public ble::Gap::EventHandler
        {
        public:
            /**
             * Construct a BLEProcess from an event queue and a ble interface.
             *
             * Call start() to initiate ble processing.
             */
            BLEProcess(events::EventQueue &event_queue, BLE &ble_interface) :
                _event_queue(event_queue),
                _ble_interface(ble_interface),
                _gap(ble_interface.gap()),
                _adv_data_builder(_adv_buffer),
                _adv_handle(ble::LEGACY_ADVERTISING_HANDLE),
                _post_init_cb()
            {
            }

            ~BLEProcess();

            /**
             * Initialize the ble interface, configure it and start advertising.
             */
            void start();

            /**
             * Close existing connections and stop the process.
             */
            void stop();

            /**
             * Subscription to the ble interface initialization event.
             *
             * @param[in] cb The callback object that will be called when the ble
             * interface is initialized.
             */
            void on_init(mbed::Callback<void(BLE&, events::EventQueue&)>* cb);

        private:
            /**
             * Sets up adverting payload and start advertising.
             *
             * This function is invoked when the ble interface is initialized.
             */
            void on_init_complete(BLE::InitializationCompleteCallbackContext *event);

            /**
             * Start the gatt client process when a connection event is received.
             * This is called by Gap to notify the application we connected
             */
            virtual void onConnectionComplete(const ble::ConnectionCompleteEvent &event);

            /**
             * Stop the gatt client process when the device is disconnected then restart
             * advertising.
             * This is called by Gap to notify the application we disconnected
             */
            virtual void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event);

            /**
             * Start the advertising process; it ends when a device connects.
             */
            void start_advertising();

            /**
             * Schedule processing of events from the BLE middleware in the event queue.
             */
            void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event);

            events::EventQueue &_event_queue;
            BLE &_ble_interface;
            ble::Gap &_gap;

            uint8_t _adv_buffer[MAX_ADVERTISING_PAYLOAD_SIZE];
            ble::AdvertisingDataBuilder _adv_data_builder;

            ble::advertising_handle_t _adv_handle;

            mbed::Callback<void(BLE&, events::EventQueue&)>* _post_init_cb;
        };
    }
}

#endif /* GATT_SERVER_EXAMPLE_BLE_PROCESS_H_ */
