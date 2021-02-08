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
#include <mbed.h>
#include "ble/BLE.h"
#include "SecurityMgr.h"
#include "pretty_printer.h"
extern "C"{
  #include "SEGGER_RTT.h"
}

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

extern const char DEVICE_NAME[] = "SecurityMgr";

using namespace MiTime::Components;
// using std::literals::chrono_literals::operator""ms;

/* Delay between steps */
static const std::chrono::milliseconds delay = 3000ms;


SecurityMgr::~SecurityMgr()
{
    _ble.onEventsToProcess(nullptr);
};

/** Start BLE interface initialisation */
void SecurityMgr::run()
{
    SEGGER_RTT_printf(0, "SecurityMgr::run(): ENTER\r\n");

    /* this will inform us off all events so we can schedule their handling
        * using our event queue */
    _ble.onEventsToProcess(makeFunctionPointer(this, &SecurityMgr::schedule_ble_events));

    /* handle gap events */
    _ble.gap().setEventHandler(this);

    if (_ble.hasInitialized()) {
        /* ble instance already initialised, skip init and start activity */
        start();
    } else {
        ble_error_t error = _ble.init(this, &SecurityMgr::on_init_complete);

        if (error) {
            SEGGER_RTT_printf(0, "Error returned by BLE::init:error=%d.\r\n", error);
            return;
        }
    }

    /* this will not return until shutdown */
    SEGGER_RTT_printf(0, "SecurityMgr::run(): BEFORE dispatch_forever()\r\n");
    _event_queue.dispatch_forever();
};

/** This is called when BLE interface is initialised and starts the demonstration */
void SecurityMgr::on_init_complete(BLE::InitializationCompleteCallbackContext *event)
{
    // SEGGER_RTT_printf(0, "SecurityMgr::on_init_complete(): ENTER\r\n");

    ble_error_t error;

    if (event->error) {
        SEGGER_RTT_printf(0, "Error during the initialisation\r\n");
        return;
    }

    /* for use by tools we print out own address and also use it
        * to seed RNG as the address is unique */
    print_local_address();

    /* This path will be used to store bonding information but will fallback
        * to storing in memory if file access fails (for example due to lack of a filesystem) */
    const char* db_path = "/fs/bt_sec_db";

    // SEGGER_RTT_printf(0, "SecurityMgr::securityManager().init() before\r\n");

    error = _ble.securityManager().init(
        /* enableBonding */ true,
        /* requireMITM */ false,
        /* iocaps */ SecurityManager::IO_CAPS_NONE,
        /* passkey */ nullptr,
        /* signing */ false,
        /* dbFilepath */ db_path
    );

    // SEGGER_RTT_printf(0, "SecurityMgr::securityManager().init() after\r\n");

    if (error) {
        SEGGER_RTT_printf(0, "Error during initialising security manager:error=%d\r\n", error);
        return;
    }

    /* This tells the stack to generate a pairingRequest event which will require
        * this application to respond before pairing can proceed. Setting it to false
        * will automatically accept pairing. */
    _ble.securityManager().setPairingRequestAuthorisation(true);

    /* this demo switches between being master and slave */
    _ble.securityManager().setHintFutureRoleReversal(true);

    /* Tell the security manager to use methods in this class to inform us
        * of any events. Class needs to implement SecurityManagerEventHandler. */
    _ble.securityManager().setSecurityManagerEventHandler(this);

    /* gap events also handled by this class */
    _ble.gap().setEventHandler(this);

    error = _ble.gap().enablePrivacy(true);
    if (error) {
        SEGGER_RTT_printf(0, "Error enabling privacy:error=%d\r\n", error);
        return;
    }

    /* continuation is in onPrivacyEnabled() - Does not seem to exist anymore */
    /* all initialisation complete, start our main activity */
    start();  // This was in onPrivacyEnabled()

    // SEGGER_RTT_printf(0, "SecurityMgr::on_init_complete(): EXIT\r\n");
};

/** Schedule processing of events from the BLE in the event queue. */
void SecurityMgr::schedule_ble_events(BLE::OnEventsToProcessCallbackContext *context)
{
    // SEGGER_RTT_printf(0, "SecurityMgr::schedule_ble_events()\r\n");
    _event_queue.call([&ble_instance = context->ble] { ble_instance.processEvents(); });
};

/** Respond to a pairing request. This will be called by the stack
 * when a pairing request arrives and expects the application to
 * call acceptPairingRequest or cancelPairingRequest */
void SecurityMgr::pairingRequest(ble::connection_handle_t connectionHandle)
{
    SEGGER_RTT_printf(0, "Pairing requested - authorising\r\n");
    _ble.securityManager().acceptPairingRequest(connectionHandle);
}

/** Inform the application of pairing */
void SecurityMgr::pairingResult(ble::connection_handle_t connectionHandle,
    SecurityManager::SecurityCompletionStatus_t result) 
{
    SEGGER_RTT_printf(0, "SecurityMgr::pairingResult(): ENTER\r\n");

    if (result == SecurityManager::SEC_STATUS_SUCCESS) {
        SEGGER_RTT_printf(0, "Pairing successful\r\n");
        _bonded = true;
    } else {
        SEGGER_RTT_printf(0, "Pairing failed\r\n");
    }

    _event_queue.call_in(
        delay,
        [this, connectionHandle] {
            _ble.gap().disconnect(connectionHandle, ble::local_disconnection_reason_t::USER_TERMINATION);
        }
    );
    SEGGER_RTT_printf(0, "SecurityMgr::pairingResult(): EXIT\r\n");
}

/** Inform the application of change in encryption status. This will be
     * communicated through the serial port */
void SecurityMgr::linkEncryptionResult(ble::connection_handle_t connectionHandle, ble::link_encryption_t result)
{
    if (result == ble::link_encryption_t::ENCRYPTED) {
        SEGGER_RTT_printf(0, "Link ENCRYPTED\r\n");
    } else if (result == ble::link_encryption_t::ENCRYPTED_WITH_MITM) {
        SEGGER_RTT_printf(0, "Link ENCRYPTED_WITH_MITM\r\n");
    } else if (result == ble::link_encryption_t::NOT_ENCRYPTED) {
        SEGGER_RTT_printf(0, "Link NOT_ENCRYPTED\r\n");
    }
}

// void SecurityMgr::onPrivacyEnabled()
// {
//     /* all initialisation complete, start our main activity */
//     start();
// }

/* Gap Event handler */

/** This is called by Gap to notify the application we connected */
void SecurityMgr::onConnectionComplete(const ble::ConnectionCompleteEvent &event)
{
    SEGGER_RTT_printf(0, "SecurityMgr::onConnectionComplete(): ENTER\r\n");

    SEGGER_RTT_printf(0, "Connected to peer: ");
    print_address(event.getPeerAddress().data());
    if (event.getPeerResolvablePrivateAddress() != ble::address_t()) {
        SEGGER_RTT_printf(0, "Peer random resolvable address: ");
        print_address(event.getPeerResolvablePrivateAddress().data());
    }

    _handle = event.getConnectionHandle();

    if (_bonded) {
        /* disconnect in 2s */
        _event_queue.call_in(
            delay,
            [this] {
                _ble.gap().disconnect(_handle, ble::local_disconnection_reason_t::USER_TERMINATION);
            }
        );
    } else {
        /* start bonding */
        ble_error_t error = _ble.securityManager().setLinkSecurity(
            _handle,
            SecurityManager::SECURITY_MODE_ENCRYPTION_NO_MITM
        );
        if (error) {
            SEGGER_RTT_printf(0, "Failed to set link security:error=%d\r\n", error);
            _ble.gap().disconnect(_handle, ble::local_disconnection_reason_t::USER_TERMINATION);
        }
    }
    SEGGER_RTT_printf(0, "SecurityMgr::onConnectionComplete(): EXIT\r\n");
}

/** This is called by Gap to notify the application we disconnected */
void SecurityMgr::onDisconnectionComplete(const ble::DisconnectionCompleteEvent &event)
{
    SEGGER_RTT_printf(0, "SecurityMgr::onDisconnectionComplete(): ENTER\r\n");

    if (_bonded) {
        /* we have connected to and bonded with the other device, from now
            * on we will use the second start function and stay in the same role
            * as peripheral or central */
        SEGGER_RTT_printf(0, "Disconnected.\r\n\r\n");
        _event_queue.call_in(delay, [this] { start(); });
    } else {
        SEGGER_RTT_printf(0, "Failed to bond.\r\n");
        _event_queue.break_dispatch();
    }
    SEGGER_RTT_printf(0, "SecurityMgr::onDisconnectionComplete(): EXIT\r\n");
}

void SecurityMgr::onScanTimeout(const ble::ScanTimeoutEvent &)
{
    /* if we failed to find the other device, abort so that we change roles */
    SEGGER_RTT_printf(0, "Haven't seen other device, switch modes.\r\n");
    _event_queue.break_dispatch();
}

void SecurityMgr::onAdvertisingEnd(const ble::AdvertisingEndEvent &event)
{
    if (!event.isConnected()) {
        SEGGER_RTT_printf(0, "No device connected to us, switch modes.\r\n");
        _event_queue.break_dispatch();
    }
}

void SecurityMgr::print_local_address()
{
    /* show what address we are using now */
    ble::own_address_type_t addr_type;
    ble::address_t addr;
    _ble.gap().getAddress(addr_type, addr);
    SEGGER_RTT_printf(0, "Device address: ");
    print_address((uint8_t *)&addr);
    static bool _seeded = false;
    if (!_seeded) {
        _seeded = true;
        /* use the address as a seed */
        uint8_t* random_data = addr.data();
        srand(*((unsigned int*)random_data));
    }
}
