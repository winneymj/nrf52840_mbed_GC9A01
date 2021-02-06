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
        /** A peripheral device will advertise and accept the connections */
        class SecurityPeripheral : public SecurityMgr {
        public:
            SecurityPeripheral(BLE &ble, events::EventQueue &event_queue)
                : SecurityMgr(ble, event_queue) { }

            /** Set up and start advertising accepting anyone */
            void start() override;

        private:
            void start_advertising();
        };
    }
}

