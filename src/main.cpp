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
#include "CurrentTimeService.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

// using mbed::callback;

// /**
//  * A Clock service that demonstrate the GattServer features.
//  *
//  * The clock service host three characteristics that model the current hour,
//  * minute and second of the clock. The value of the second characteristic is
//  * incremented automatically by the system.
//  *
//  * A client can subscribe to updates of the clock characteristics and get
//  * notified when one of the value is changed. Clients can also change value of
//  * the second, minute and hour characteristric.
//  */
// class ClockService {
//     typedef ClockService Self;

// public:
//     ClockService() :
//         _currentTimeCharacteristic(GattCharacteristic::UUID_CURRENT_TIME_CHAR, {0}),
//         _ct_uuid(GattService::UUID_CURRENT_TIME_SERVICE),
//         _charsTable(),
//         _clock_service(
//             /* uuid */ _ct_uuid,
//             /* characteristics */ _charsTable,
//             /* numCharacteristics */ sizeof(_charsTable) / sizeof(GattCharacteristic*)),
//         _server(NULL),
//         _event_queue(NULL)
//     {
//         _charsTable[0] = {&_currentTimeCharacteristic};

//         // update internal pointers (value, descriptors and characteristics array)
//         // _clock_characteristics[0] = &_hour_char;
//         // _clock_characteristics[1] = &_minute_char;
//         // _clock_characteristics[2] = &_second_char;

//         // setup authorization handlers
//         // _hour_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
//         // _minute_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
//         // _second_char.setWriteAuthorizationCallback(this, &Self::authorize_client_write);
//     }



//     void start(BLE &ble_interface, events::EventQueue &event_queue)
//     {
//          if (_event_queue) {
//             return;
//         }

//         _server = &ble_interface.gattServer();
//         _event_queue = &event_queue;

//         // register the service
//         SEGGER_RTT_printf(0, "Adding demo service\r\n");
//         ble_error_t err = _server->addService(_clock_service);

//         if (err) {
//             SEGGER_RTT_printf(0, "Error %u during demo service registration.\r\n", err);
//             return;
//         }

//         // read write handler
//         _server->onDataSent(as_cb(&Self::when_data_sent));
//         _server->onDataWritten(as_cb(&Self::when_data_written));
//         _server->onDataRead(as_cb(&Self::when_data_read));

//         // updates subscribtion handlers
//         _server->onUpdatesEnabled(as_cb(&Self::when_update_enabled));
//         _server->onUpdatesDisabled(as_cb(&Self::when_update_disabled));
//         _server->onConfirmationReceived(as_cb(&Self::when_confirmation_received));

//         // print the handles
//         SEGGER_RTT_printf(0, "clock service registered\r\n");
//         SEGGER_RTT_printf(0, "service handle: %u\r\n", _clock_service.getHandle());
//         // SEGGER_RTT_printf(0, "\thour characteristic value handle %u\r\n", _hour_char.getValueHandle());
//         // SEGGER_RTT_printf(0, "\tminute characteristic value handle %u\r\n", _minute_char.getValueHandle());
//         // SEGGER_RTT_printf(0, "\tsecond characteristic value handle %u\r\n", _second_char.getValueHandle());

//         _event_queue->call_every(1000 /* ms */, callback(this, &Self::increment_second));
//     }

// private:

//     /**
//      * Handler called when a notification or an indication has been sent.
//      */
//     void when_data_sent(unsigned count)
//     {
//         SEGGER_RTT_printf(0, "sent %u updates\r\n", count);
//     }

//     /**
//      * Handler called after an attribute has been written.
//      */
//     void when_data_written(const GattWriteCallbackParams *e)
//     {
//         SEGGER_RTT_printf(0, "data written:\r\n");
//         SEGGER_RTT_printf(0, "\tconnection handle: %u\r\n", e->connHandle);
//         SEGGER_RTT_printf(0, "\tattribute handle: %u", e->handle);
//         // if (e->handle == _hour_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (hour characteristic)\r\n");
//         // } else if (e->handle == _minute_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (minute characteristic)\r\n");
//         // } else if (e->handle == _second_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (second characteristic)\r\n");
//         // } else {
//         //     SEGGER_RTT_printf(0, "\r\n");
//         // }
//         SEGGER_RTT_printf(0, "\twrite operation: %u\r\n", e->writeOp);
//         SEGGER_RTT_printf(0, "\toffset: %u\r\n", e->offset);
//         SEGGER_RTT_printf(0, "\tlength: %u\r\n", e->len);
//         SEGGER_RTT_printf(0, "\t data: ");

//         for (size_t i = 0; i < e->len; ++i) {
//             SEGGER_RTT_printf(0, "%02X", e->data[i]);
//         }

//         SEGGER_RTT_printf(0, "\r\n");
//     }

//     /**
//      * Handler called after an attribute has been read.
//      */
//     void when_data_read(const GattReadCallbackParams *e)
//     {
//         SEGGER_RTT_printf(0, "data read:\r\n");
//         SEGGER_RTT_printf(0, "\tconnection handle: %u\r\n", e->connHandle);
//         SEGGER_RTT_printf(0, "\tattribute handle: %u", e->handle);
//         // if (e->handle == _hour_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (hour characteristic)\r\n");
//         // } else if (e->handle == _minute_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (minute characteristic)\r\n");
//         // } else if (e->handle == _second_char.getValueHandle()) {
//         //     SEGGER_RTT_printf(0, " (second characteristic)\r\n");
//         // } else {
//         //     SEGGER_RTT_printf(0, "\r\n");
//         // }
//     }

//     /**
//      * Handler called after a client has subscribed to notification or indication.
//      *
//      * @param handle Handle of the characteristic value affected by the change.
//      */
//     void when_update_enabled(GattAttribute::Handle_t handle)
//     {
//         SEGGER_RTT_printf(0, "update enabled on handle %d\r\n", handle);
//     }

//     /**
//      * Handler called after a client has cancelled his subscription from
//      * notification or indication.
//      *
//      * @param handle Handle of the characteristic value affected by the change.
//      */
//     void when_update_disabled(GattAttribute::Handle_t handle)
//     {
//         SEGGER_RTT_printf(0, "update disabled on handle %d\r\n", handle);
//     }

//     /**
//      * Handler called when an indication confirmation has been received.
//      *
//      * @param handle Handle of the characteristic value that has emitted the
//      * indication.
//      */
//     void when_confirmation_received(GattAttribute::Handle_t handle)
//     {
//         SEGGER_RTT_printf(0, "confirmation received on handle %d\r\n", handle);
//     }

//     /**
//      * Handler called when a write request is received.
//      *
//      * This handler verify that the value submitted by the client is valid before
//      * authorizing the operation.
//      */
//     void authorize_client_write(GattWriteAuthCallbackParams *e)
//     {
//         SEGGER_RTT_printf(0, "characteristic %u write authorization\r\n", e->handle);

//         if (e->offset != 0) {
//             SEGGER_RTT_printf(0, "Error invalid offset\r\n");
//             e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
//             return;
//         }

//         if (e->len != 1) {
//             SEGGER_RTT_printf(0, "Error invalid len\r\n");
//             e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
//             return;
//         }

//         // if ((e->data[0] >= 60) ||
//         //     ((e->data[0] >= 24) && (e->handle == _hour_char.getValueHandle()))) {
//         //     SEGGER_RTT_printf(0, "Error invalid data\r\n");
//         //     e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
//         //     return;
//         // }

//         e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
//     }

//     /**
//      * Increment the second counter.
//      */
//     void increment_second(void)
//     {
//         uint8_t second = 0;
//         // ble_error_t err = _second_char.get(*_server, second);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "read of the second value returned error %u\r\n", err);
//         //     return;
//         // }

//         second = (second + 1) % 60;

//         // err = _second_char.set(*_server, second);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "write of the second value returned error %u\r\n", err);
//         //     return;
//         // }

//         if (second == 0) {
//             increment_minute();
//         }
//     }

//     /**
//      * Increment the minute counter.
//      */
//     void increment_minute(void)
//     {
//         uint8_t minute = 0;
//         // ble_error_t err = _minute_char.get(*_server, minute);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "read of the minute value returned error %u\r\n", err);
//         //     return;
//         // }

//         minute = (minute + 1) % 60;

//         // err = _minute_char.set(*_server, minute);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "write of the minute value returned error %u\r\n", err);
//         //     return;
//         // }

//         if (minute == 0) {
//             increment_hour();
//         }
//     }

//     /**
//      * Increment the hour counter.
//      */
//     void increment_hour(void)
//     {
//         uint8_t hour = 0;
//         // ble_error_t err = _hour_char.get(*_server, hour);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "read of the hour value returned error %u\r\n", err);
//         //     return;
//         // }

//         hour = (hour + 1) % 24;

//         // err = _hour_char.set(*_server, hour);
//         // if (err) {
//         //     SEGGER_RTT_printf(0, "write of the hour value returned error %u\r\n", err);
//         //     return;
//         // }
//     }

// private:
//     /**
//      * Helper that construct an event handler from a member function of this
//      * instance.
//      */
//     template<typename Arg>
//     FunctionPointerWithContext<Arg> as_cb(void (Self::*member)(Arg))
//     {
//         return makeFunctionPointer(this, member);
//     }

//     /**
//      * Read, Write, Notify, Indicate  Characteristic declaration helper.
//      *
//      * @tparam T type of data held by the characteristic.
//      */
//     template<typename T>
//     class ReadWriteNotifyIndicateCharacteristic : public GattCharacteristic {
//     public:
//         /**
//          * Construct a characteristic that can be read or written and emit
//          * notification or indication.
//          *
//          * @param[in] uuid The UUID of the characteristic.
//          * @param[in] initial_value Initial value contained by the characteristic.
//          */
//         ReadWriteNotifyIndicateCharacteristic(const UUID & uuid, const T& initial_value) :
//             GattCharacteristic(
//                 /* UUID */ uuid,
//                 /* Initial value */ (uint8_t *)&_value,
//                 /* Value size */ sizeof(_value),
//                 /* Value capacity */ sizeof(_value),
//                 /* Properties */ GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_READ |
//                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE |
//                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_NOTIFY |
//                                 GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_INDICATE,
//                 /* Descriptors */ NULL,
//                 /* Num descriptors */ 0,
//                 /* variable len */ false
//             ),
//             _value(initial_value)
//         {
//         }

//         /**
//          * Get the value of this characteristic.0
//          *
//          * @param[in] server GattServer instance that contain the characteristic
//          * value.
//          * @param[in] dst Variable that will receive the characteristic value.
//          *
//          * @return BLE_ERROR_NONE in case of success or an appropriate error code.
//          */
//         ble_error_t get(GattServer &server, T& dst) const
//         {
//             uint16_t value_length = sizeof(dst);
//             return server.read(getValueHandle(), &dst, &value_length);
//         }

//         /**
//          * Assign a new value to this characteristic.
//          *
//          * @param[in] server GattServer instance that will receive the new value.
//          * @param[in] value The new value to set.
//          * @param[in] local_only Flag that determine if the change should be kept
//          * locally or forwarded to subscribed clients.
//          */
//         ble_error_t set(
//             GattServer &server, const T& value, bool local_only = false
//         ) const {
//             return server.write(getValueHandle(), &value, sizeof(value), local_only);
//         }

//     private:
//         T _value;
//     };

//     // ReadWriteNotifyIndicateCharacteristic<uint8_t> _hour_char;
//     // ReadWriteNotifyIndicateCharacteristic<uint8_t> _minute_char;
//     // ReadWriteNotifyIndicateCharacteristic<uint8_t> _second_char;

//     typedef struct
//     {
//         uint16_t year;
//         uint8_t month;
//         uint8_t dayofmonth;
//         uint8_t hour;
//         uint8_t minute;
//         uint8_t second;
//         uint8_t millis;
//         uint8_t reason;
//     } CtsData;

//     ReadWriteNotifyIndicateCharacteristic<CtsData> _currentTimeCharacteristic;

//     // list of the characteristics of the clock service
//     // GattCharacteristic* _clock_characteristics[3];
//     // GattCharacteristic _currentTimeCharacteristic;

//     UUID _ct_uuid;

//     CtsData _valueBytes;

//     GattCharacteristic *_charsTable[1];

//     // demo service
//     GattService _clock_service;

//     GattServer* _server;

//     events::EventQueue *_event_queue;
// };

int main() {
    BLE &ble_interface = BLE::Instance();
    events::EventQueue event_queue;
    CurrentTimeService demo_service;
    BLEProcess ble_process(event_queue, ble_interface);

    ble_process.on_init(callback(&demo_service, &CurrentTimeService::start));

    // bind the event queue to the ble interface, initialize the interface
    // and start advertising
    ble_process.start();

    return 0;
}

