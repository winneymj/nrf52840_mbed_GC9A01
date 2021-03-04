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

using namespace Mytime::Controllers;
using mbed::callback;

void CurrentTimeService::start(BLE &ble_interface, events::EventQueue &event_queue)
{
        if (_event_queue) {
        return;
    }

    _server = &ble_interface.gattServer();
    _event_queue = &event_queue;

    // register the service
    SEGGER_RTT_printf(0, "Adding demo service\r\n");
    ble_error_t err = _server->addService(_clock_service);

    if (err) {
        SEGGER_RTT_printf(0, "Error %u during demo service registration.\r\n", err);
        return;
    }

    // read write handler
    _server->onDataSent(as_cb(&Self::when_data_sent));
    _server->onDataWritten(as_cb(&Self::when_data_written));
    _server->onDataRead(as_cb(&Self::when_data_read));

    // updates subscribtion handlers
    _server->onUpdatesEnabled(as_cb(&Self::when_update_enabled));
    _server->onUpdatesDisabled(as_cb(&Self::when_update_disabled));
    _server->onConfirmationReceived(as_cb(&Self::when_confirmation_received));

    // print the handles
    SEGGER_RTT_printf(0, "clock service registered\r\n");
    SEGGER_RTT_printf(0, "service handle: %u\r\n", _clock_service.getHandle());
    // SEGGER_RTT_printf(0, "\thour characteristic value handle %u\r\n", _hour_char.getValueHandle());
    // SEGGER_RTT_printf(0, "\tminute characteristic value handle %u\r\n", _minute_char.getValueHandle());
    // SEGGER_RTT_printf(0, "\tsecond characteristic value handle %u\r\n", _second_char.getValueHandle());

    _event_queue->call_every(1000 /* ms */, callback(this, &Self::increment_second));
}

/**
 * Handler called when a notification or an indication has been sent.
 */
void CurrentTimeService::when_data_sent(unsigned count)
{
    SEGGER_RTT_printf(0, "sent %u updates\r\n", count);
}

/**
 * Handler called after an attribute has been written.
 */
void CurrentTimeService::when_data_written(const GattWriteCallbackParams *e)
{
    SEGGER_RTT_printf(0, "data written:\r\n");
    SEGGER_RTT_printf(0, "\tconnection handle: %u\r\n", e->connHandle);
    SEGGER_RTT_printf(0, "\tattribute handle: %u\n", e->handle);

    BLE_DateTime result;
    memcpy(&result, (void *)e->data, (e->len > sizeof(BLE_DateTime) ? sizeof(BLE_DateTime): e->len));

    SEGGER_RTT_printf(0, "Received data: %d-%d-%d %d:%d:%d\n", 
                        result.day, result.month, result.year,
                        result.hours, result.minutes, result.seconds);

    _dateTimeController.SetTime(result.year, result.month, result.day, 0, result.hours, result.minutes, result.seconds);

    SEGGER_RTT_printf(0, "\twrite operation: %u\r\n", e->writeOp);
    SEGGER_RTT_printf(0, "\toffset: %u\r\n", e->offset);
    SEGGER_RTT_printf(0, "\tlength: %u\r\n", e->len);
    SEGGER_RTT_printf(0, "\t data: ");

    for (size_t i = 0; i < e->len; ++i) {
        SEGGER_RTT_printf(0, "%02X", e->data[i]);
    }
    SEGGER_RTT_printf(0, "\r\n");

    time_t seconds = _dateTimeController.CurrentDateTime();
    SEGGER_RTT_printf(0, "Time as a basic string = %s\n", ctime(&seconds));
}

/**
 * Handler called after an attribute has been read.
 */
void CurrentTimeService::when_data_read(const GattReadCallbackParams *e)
{
    SEGGER_RTT_printf(0, "data read:\r\n");
    SEGGER_RTT_printf(0, "\tconnection handle: %u\r\n", e->connHandle);
    SEGGER_RTT_printf(0, "\tattribute handle: %u", e->handle);
    // if (e->handle == _hour_char.getValueHandle()) {
    //     SEGGER_RTT_printf(0, " (hour characteristic)\r\n");
    // } else if (e->handle == _minute_char.getValueHandle()) {
    //     SEGGER_RTT_printf(0, " (minute characteristic)\r\n");
    // } else if (e->handle == _second_char.getValueHandle()) {
    //     SEGGER_RTT_printf(0, " (second characteristic)\r\n");
    // } else {
    //     SEGGER_RTT_printf(0, "\r\n");
    // }
}

/**
 * Handler called after a client has subscribed to notification or indication.
 *
 * @param handle Handle of the characteristic value affected by the change.
 */
void CurrentTimeService::when_update_enabled(GattAttribute::Handle_t handle)
{
    SEGGER_RTT_printf(0, "update enabled on handle %d\r\n", handle);
}

/**
 * Handler called after a client has cancelled his subscription from
 * notification or indication.
 *
 * @param handle Handle of the characteristic value affected by the change.
 */
void CurrentTimeService::when_update_disabled(GattAttribute::Handle_t handle)
{
    SEGGER_RTT_printf(0, "update disabled on handle %d\r\n", handle);
}

/**
 * Handler called when an indication confirmation has been received.
 *
 * @param handle Handle of the characteristic value that has emitted the
 * indication.
 */
void CurrentTimeService::when_confirmation_received(GattAttribute::Handle_t handle)
{
    SEGGER_RTT_printf(0, "confirmation received on handle %d\r\n", handle);
}

/**
 * Handler called when a write request is received.
 *
 * This handler verify that the value submitted by the client is valid before
 * authorizing the operation.
 */
void CurrentTimeService::authorize_client_write(GattWriteAuthCallbackParams *e)
{
    SEGGER_RTT_printf(0, "characteristic %u write authorization\r\n", e->handle);

    if (e->offset != 0) {
        SEGGER_RTT_printf(0, "Error invalid offset\r\n");
        e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_OFFSET;
        return;
    }

    if (e->len != 1) {
        SEGGER_RTT_printf(0, "Error invalid len\r\n");
        e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_INVALID_ATT_VAL_LENGTH;
        return;
    }

    // if ((e->data[0] >= 60) ||
    //     ((e->data[0] >= 24) && (e->handle == _hour_char.getValueHandle()))) {
    //     SEGGER_RTT_printf(0, "Error invalid data\r\n");
    //     e->authorizationReply = AUTH_CALLBACK_REPLY_ATTERR_WRITE_NOT_PERMITTED;
    //     return;
    // }

    e->authorizationReply = AUTH_CALLBACK_REPLY_SUCCESS;
}

/**
 * Increment the second counter.
 */
void CurrentTimeService::increment_second(void)
{
    uint8_t second = 0;
    // ble_error_t err = _second_char.get(*_server, second);
    // if (err) {
    //     SEGGER_RTT_printf(0, "read of the second value returned error %u\r\n", err);
    //     return;
    // }

    second = (second + 1) % 60;

    // err = _second_char.set(*_server, second);
    // if (err) {
    //     SEGGER_RTT_printf(0, "write of the second value returned error %u\r\n", err);
    //     return;
    // }

    if (second == 0) {
        increment_minute();
    }
}

/**
 * Increment the minute counter.
 */
void CurrentTimeService::increment_minute(void)
{
    uint8_t minute = 0;
    // ble_error_t err = _minute_char.get(*_server, minute);
    // if (err) {
    //     SEGGER_RTT_printf(0, "read of the minute value returned error %u\r\n", err);
    //     return;
    // }

    minute = (minute + 1) % 60;

    // err = _minute_char.set(*_server, minute);
    // if (err) {
    //     SEGGER_RTT_printf(0, "write of the minute value returned error %u\r\n", err);
    //     return;
    // }

    if (minute == 0) {
        increment_hour();
    }
}

/**
 * Increment the hour counter.
 */
void CurrentTimeService::increment_hour(void)
{
    uint8_t hour = 0;
    // ble_error_t err = _hour_char.get(*_server, hour);
    // if (err) {
    //     SEGGER_RTT_printf(0, "read of the hour value returned error %u\r\n", err);
    //     return;
    // }

    hour = (hour + 1) % 24;

    // err = _hour_char.set(*_server, hour);
    // if (err) {
    //     SEGGER_RTT_printf(0, "write of the hour value returned error %u\r\n", err);
    //     return;
    // }
}
