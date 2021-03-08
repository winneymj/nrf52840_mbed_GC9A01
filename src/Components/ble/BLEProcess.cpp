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

#include <stdint.h>
#include "pretty_printer.h"

#include <events/mbed_events.h>
#include "platform/Callback.h"
#include "platform/NonCopyable.h"

#include "ble/BLE.h"
#include "ble/Gap.h"
#include "gap/AdvertisingDataParser.h"
#include "ble/common/FunctionPointerWithContext.h"
#include "BLEProcess.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

/**
 * Handle initialization and shutdown of the BLE Instance.
 *
 * Setup advertising payload and manage advertising state.
 * Delegate to GattClientProcess once the connection is established.
 */
BLEProcess::~BLEProcess()
{
    stop();
}

    /**
     * Initialize the ble interface, configure it and start advertising.
     */
void BLEProcess::start()
{
    SEGGER_RTT_printf(0, "BLEProcess::start: START\r\n");

    if (_ble_interface.hasInitialized()) {
        SEGGER_RTT_printf(0, "Error: the ble instance has already been initialized.\r\n");
        return;
    }

    /* handle gap events */
    _gap.setEventHandler(this);

    /* This will inform us off all events so we can schedule their handling
        * using our event queue */
    _ble_interface.onEventsToProcess(
        makeFunctionPointer(this, &BLEProcess::schedule_ble_events)
    );

    SEGGER_RTT_printf(0, "BLEProcess::start: _ble_interface.init()\r\n");
    ble_error_t error = _ble_interface.init(this, &BLEProcess::on_init_complete);

    if (error) {
        print_error(error, "Error returned by BLE::init.\r\n");
        return;
    }

    SEGGER_RTT_printf(0, "BLEProcess::start: dispatch_forever()\r\n");
    // Process the event queue.
    _event_queue.dispatch_forever();

    return;
}

/**
 * Close existing connections and stop the process.
 */
void BLEProcess::stop()
{
    if (_ble_interface.hasInitialized()) {
        _ble_interface.shutdown();
        SEGGER_RTT_printf(0, "Ble process stopped.");
    }
}

/**
 * Subscription to the ble interface initialization event.
 *
 * @param[in] cb The callback object that will be called when the ble
 * interface is initialized.
 */
void BLEProcess::on_init(mbed::Callback<void(BLE&, events::EventQueue&)>* cb)
{
    _post_init_cb = cb;
}

/**
 * Sets up adverting payload and start advertising.
 *
 * This function is invoked when the ble interface is initialized.
 */
void BLEProcess::on_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    if (event->error) {
        print_error(event->error, "Error during the initialisation\r\n");
        return;
    }

    SEGGER_RTT_printf(0, "Ble instance initialized\r\n");

    /* All calls are serialised on the user thread through the event queue */
    _event_queue.call(this, &BLEProcess::start_advertising);

    int loop = 0;
    while (_post_init_cb[loop])
    {
        if (_post_init_cb[loop])
        {
            _post_init_cb[loop](_ble_interface, _event_queue);
        }
        loop++;
    };
}

/**
 * Start the gatt client process when a connection event is received.
 * This is called by Gap to notify the application we connected
 */
void BLEProcess::onConnectionComplete(
    const ble::ConnectionCompleteEvent &event
) {
    if (event.getStatus() == BLE_ERROR_NONE) {
        SEGGER_RTT_printf(0, "Connected.\r\n");
    } else {
        SEGGER_RTT_printf(0, "Failed to connect\r\n");
        _event_queue.call(this, &BLEProcess::start_advertising);
    }
}

/**
 * Stop the gatt client process when the device is disconnected then restart
 * advertising.
 * This is called by Gap to notify the application we disconnected
 */
void BLEProcess::onDisconnectionComplete(
    const ble::DisconnectionCompleteEvent &event
) {
    SEGGER_RTT_printf(0, "Disconnected.\r\n");
    _event_queue.call(this, &BLEProcess::start_advertising);
}

/**
 * Start the advertising process; it ends when a device connects.
 */
void BLEProcess::start_advertising()
{
    ble_error_t error;

    ble::AdvertisingParameters adv_params;

    error = _gap.setAdvertisingParameters(_adv_handle, adv_params);

    if (error) {
        SEGGER_RTT_printf(0, "_ble.gap().setAdvertisingParameters() failed\r\n");
        return;
    }

    _adv_data_builder.clear();
    _adv_data_builder.setFlags();
    _adv_data_builder.setName(DEVICE_NAME);

    /* Set payload for the set */
    error = _gap.setAdvertisingPayload(
        _adv_handle, _adv_data_builder.getAdvertisingData()
    );

    if (error) {
        print_error(error, "Gap::setAdvertisingPayload() failed\r\n");
        return;
    }

    error = _gap.startAdvertising(_adv_handle);

    if (error) {
        print_error(error, "Gap::startAdvertising() failed\r\n");
        return;
    }

    SEGGER_RTT_printf(0, "Advertising started.\r\n");
}

/**
 * Schedule processing of events from the BLE middleware in the event queue.
 */
void BLEProcess::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *event)
{
    _event_queue.call(mbed::callback(&event->ble, &BLE::processEvents));
}
