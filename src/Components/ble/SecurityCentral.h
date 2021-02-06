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
#include "SecurityMgr.h"

namespace MiTime {
    namespace Components {
        /** A central device will scan and connect to a peer. */
        class SecurityCentral : public SecurityMgr {
        public:
            SecurityCentral(BLE &ble, events::EventQueue &event_queue)
                : SecurityMgr(ble, event_queue) {}

            /** start scanning and attach a callback that will handle advertisements
             *  and scan requests responses */
            void start() override;

            /* helper functions */
        private:
            bool start_scanning();

        private:
            /* Event handler */

            /** Look at scan payload to find a peer device and connect to it */
            void onAdvertisingReport(const ble::AdvertisingReportEvent &event) override;

        private:
            bool _is_connecting = false;
        };
    }
}
