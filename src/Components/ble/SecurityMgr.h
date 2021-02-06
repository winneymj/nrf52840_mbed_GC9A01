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
#pragma once
#include <events/mbed_events.h>
#include <mbed.h>
#include <NonCopyable.h>
#include "ble/BLE.h"

/** This example demonstrates all the basic setup required for pairing and setting
 *  up link security both as a central and peripheral. It also demonstrates privacy
 *  features in Gap. It shows how to use private addresses when advertising and
 *  connecting and how filtering ties in with these operations.
 *
 *  The application will start by repeatedly trying to connect to the same
 *  application running on another board. It will do this by advertising and
 *  scanning for random intervals waiting until the difference in intervals
 *  between the boards will make them meet when one is advertising and the
 *  other scanning.
 *
 *  Two devices will be operating using random resolvable addresses. The
 *  applications will connect to the peer and pair. It will attempt bonding
 *  to store the IRK that resolve the peer. Subsequent connections will
 *  turn on filtering based on stored IRKs.
 */

/** Base class for both peripheral and central. The same class that provides
 *  the logic for the application also implements the SecurityManagerEventHandler
 *  which is the interface used by the Security Manager to communicate events
 *  back to the applications. You can provide overrides for a selection of events
 *  your application is interested in.
 */
namespace MiTime {
    namespace Components {
        class SecurityMgr : private mbed::NonCopyable<SecurityMgr>,
                            public SecurityManager::EventHandler,
                            public ble::Gap::EventHandler
        {
        public:
            SecurityMgr(BLE &ble, events::EventQueue &event_queue) :
                _ble(ble), _event_queue(event_queue) { };

            virtual ~SecurityMgr();

            /** Start BLE interface initialisation */
            void run();

            /** Override to start chosen activity when the system starts */
            virtual void start() = 0;

            /* callbacks */

            /** This is called when BLE interface is initialised and starts the demonstration */
            void on_init_complete(BLE::InitializationCompleteCallbackContext *event);

            /** Schedule processing of events from the BLE in the event queue. */
            void schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context);

        private:
            /* SecurityManager Event handler */

            /** Respond to a pairing request. This will be called by the stack
             * when a pairing request arrives and expects the application to
             * call acceptPairingRequest or cancelPairingRequest */
            void pairingRequest(ble::connection_handle_t connectionHandle) override;

            /** Inform the application of pairing */
            void pairingResult(ble::connection_handle_t connectionHandle,
                ble::SecurityManager::SecurityCompletionStatus_t result) override;

            /** Inform the application of change in encryption status. This will be
             * communicated through the serial port */
            void linkEncryptionResult(ble::connection_handle_t connectionHandle, ble::link_encryption_t result) override;

            // void onPrivacyEnabled() override;

            /* Gap Event handler */

            /** This is called by Gap to notify the application we connected */
            void onConnectionComplete(const ble::ConnectionCompleteEvent &event) override;

            /** This is called by Gap to notify the application we disconnected */
            void onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event) override;

            void onScanTimeout(const ble::ScanTimeoutEvent &) override;

            void onAdvertisingEnd(const ble::AdvertisingEndEvent &event) override;

        private:
            void print_local_address();

        protected:
            BLE &_ble;
            events::EventQueue &_event_queue;
            ble::connection_handle_t _handle = 0;
            bool _bonded = false;
        };
    }
}
