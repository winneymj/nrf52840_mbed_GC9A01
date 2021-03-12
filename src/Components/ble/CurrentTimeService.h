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

#ifndef __CURRENT_TIME_SERVICE_H__
#define __CURRENT_TIME_SERVICE_H__

#include "mbed.h"
#include "platform/Callback.h"
#include "events/EventQueue.h"
#include "platform/NonCopyable.h"

#include "ble/GattServer.h"
#include "components/datetime/DateTimeController.h"
#include "BLEProcess.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

/**
 * A Clock service that demonstrate the GattServer features.
 *
 * The clock service host three characteristics that model the current hour,
 * minute and second of the clock. The value of the second characteristic is
 * incremented automatically by the system.
 *
 * A client can subscribe to updates of the clock characteristics and get
 * notified when one of the value is changed. Clients can also change value of
 * the second, minute and hour characteristric.
 */
namespace Mytime {
    namespace Controllers {

        /**
         * Read, Write, Notify, Indicate  Characteristic declaration helper.
         *
         * @tparam T type of data held by the characteristic.
         */
        template<typename T>
        class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
        public:
            /**
             * Construct a characteristic that can be read or written and emit
             * notification or indication.
             *
             * @param[in] uuid The UUID of the characteristic.
             * @param[in] initial_value Initial value contained by the characteristic.
             */
            ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
                GattCharacteristic(
                    /* UUID */ uuid,
                    /* Initial value */ (uint8_t *)&_value,
                    /* Value size */ sizeof(_value),
                    /* Value capacity */ sizeof(_value),
                    /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
                                    GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
                    /* Descriptors */ NULL,
                    /* Num descriptors */ 0,
                    /* variable len */ false
                ),
                _value(initial_value)
            {
                SEGGER_RTT_printf(0, "ReadWriteNotifyIndicateCharacteristic: size= %d\r\n", sizeof(_value));
            }

            /**
             * Get the value of this characteristic.0
             *
             * @param[in] server GattServer instance that contain the characteristic
             * value.
             * @param[in] dst Variable that will receive the characteristic value.
             *
             * @return BLE_ERROR_NONE in case of success or an appropriate error code.
             */
            ble_error_t get(GattServer &server, T& dst) const
            {
                uint16_t value_length = sizeof(dst);
                return server.read(getValueHandle(), &dst, &value_length);
            }

            /**
             * Assign a new value to this characteristic.
             *
             * @param[in] server GattServer instance that will receive the new value.
             * @param[in] value The new value to set.
             * @param[in] local_only Flag that determine if the change should be kept
             * locally or forwarded to subscribed clients.
             */
            ble_error_t set(
                GattServer &server, const T& value, bool local_only = false
            ) const {
                return server.write(getValueHandle(), &value, sizeof(value), local_only);
            }

        private:
            T _value;
        };


        /**
         * Notify Characteristic declaration helper.
         *
         * @tparam T type of data held by the characteristic.
         */
        template<typename T>
        class NotifyCharacteristic : public GattCharacteristic {
        public:
            /**
             * Construct a characteristic that can be read or written and emit
             * notification or indication.
             *
             * @param[in] uuid The UUID of the characteristic.
             * @param[in] initial_value Initial value contained by the characteristic.
             */
            NotifyCharacteristic(const UUID & uuid, const T& initial_value) :
                GattCharacteristic(
                    /* UUID */ uuid,
                    /* Initial value */ (uint8_t *)&_value,
                    /* Value size */ sizeof(_value),
                    /* Value capacity */ sizeof(_value),
                    /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY,
                    /* Descriptors */ NULL,
                    /* Num descriptors */ 0,
                    /* variable len */ false
                ),
                _value(initial_value)
            {
                SEGGER_RTT_printf(0, "NotifyCharacteristic: size= %d\r\n", sizeof(_value));
            }

            /**
             * Get the value of this characteristic.0
             *
             * @param[in] server GattServer instance that contain the characteristic
             * value.
             * @param[in] dst Variable that will receive the characteristic value.
             *
             * @return BLE_ERROR_NONE in case of success or an appropriate error code.
             */
            ble_error_t get(GattServer &server, T& dst) const
            {
                uint16_t value_length = sizeof(dst);
                return server.read(getValueHandle(), &dst, &value_length);
            }

            /**
             * Assign a new value to this characteristic.
             *
             * @param[in] server GattServer instance that will receive the new value.
             * @param[in] value The new value to set.
             * @param[in] local_only Flag that determine if the change should be kept
             * locally or forwarded to subscribed clients.
             */
            ble_error_t set(
                GattServer &server, const T& value, bool local_only = false
            ) const {
                return server.write(getValueHandle(), (const uint8_t *)&value, sizeof(value), local_only);
            }

        private:
            T _value;
        };

        class CurrentTimeService {
            typedef CurrentTimeService Self;

        public:
            CurrentTimeService(DateTimeController &dateTimeController) :
                _currentTimeCharacteristic(GattCharacteristic::UUID_CURRENT_TIME_CHAR, {0}),
                _ct_uuid(GattService::UUID_CURRENT_TIME_SERVICE),
                _charsTable(),
                _clock_service(
                    /* uuid */ _ct_uuid,
                    /* characteristics */ _charsTable,
                    /* numCharacteristics */ sizeof(_charsTable) / sizeof(GattCharacteristic*)),
                _server(NULL),
                _event_queue(NULL),
                _dateTimeController(dateTimeController)
            {
                _charsTable[0] = {&_currentTimeCharacteristic};
            }

            void start(BLE &ble_interface, events::EventQueue &event_queue);

        private:

            /**
             * Handler called when a notification or an indication has been sent.
             */
            void when_data_sent(unsigned count);

            /**
             * Handler called after an attribute has been written.
             */
            void when_data_written(const GattWriteCallbackParams *e);

            /**
             * Handler called after an attribute has been read.
             */
            void when_data_read(const GattReadCallbackParams *e);

            /**
             * Handler called after a client has subscribed to notification or indication.
             *
             * @param handle Handle of the characteristic value affected by the change.
             */
            void when_update_enabled(GattAttribute::Handle_t handle);

            /**
             * Handler called after a client has cancelled his subscription from
             * notification or indication.
             *
             * @param handle Handle of the characteristic value affected by the change.
             */
            void when_update_disabled(GattAttribute::Handle_t handle);

            /**
             * Handler called when an indication confirmation has been received.
             *
             * @param handle Handle of the characteristic value that has emitted the
             * indication.
             */
            void when_confirmation_received(GattAttribute::Handle_t handle);

            /**
             * Handler called when a write request is received.
             *
             * This handler verify that the value submitted by the client is valid before
             * authorizing the operation.
             */
            void authorize_client_write(GattWriteAuthCallbackParams *e);

            /**
             * Increment the second counter.
             */
            void increment_second(void);

            /**
             * Increment the minute counter.
             */
            void increment_minute(void);

            /**
             * Increment the hour counter.
             */
            void increment_hour(void);

        private:
            /**
             * Helper that construct an event handler from a member function of this
             * instance.
             */
            template<typename Arg>
            FunctionPointerWithContext<Arg> as_cb(void (Self::*member)(Arg))
            {
                return makeFunctionPointer(this, member);
            }

            typedef struct {
                uint16_t year;
                uint8_t  month;
                uint8_t  day;
                uint8_t  hours;
                uint8_t  minutes;
                uint8_t  seconds;
                uint8_t  dayOfWeek;
                uint8_t  fractions256;
                uint8_t  adjustReason;
                uint8_t  dst;
            } BLE_DateTime;
            
            ReadWriteNotifyIndicateCharacteristic<BLE_DateTime> _currentTimeCharacteristic;

            UUID _ct_uuid;

            GattCharacteristic *_charsTable[1];

            // demo service
            GattService _clock_service;

            GattServer* _server;

            events::EventQueue *_event_queue;

            DateTimeController &_dateTimeController;
        };
    }
}
#endif // __CURRENT_TIME_SERVICE_H__