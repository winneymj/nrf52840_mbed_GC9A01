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

#include "AlertNotificationService.h"
#include "NotificationManager.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

using namespace Mytime::Controllers;

void AlertNotificationService::start(BLE &ble_interface, events::EventQueue &event_queue)
{
    SEGGER_RTT_printf(0, "AlertNotificationService: START\r\n");
    if (_event_queue) {
        return;
    }

    _server = &ble_interface.gattServer();
    _event_queue = &event_queue;

    // register the service
    SEGGER_RTT_printf(0, "Adding _notification_service service\r\n");
    ble_error_t err = _server->addService(_notification_service);

    if (err) {
        SEGGER_RTT_printf(0, "Error %u during AlertNotificationService service registration.\r\n", err);
        return;
    }

    // read write handler
    // _server->onDataSent(as_cb(&Self::when_data_sent));
    _server->onDataWritten(as_cb(&Self::when_data_written));
    // _server->onDataRead(as_cb(&Self::when_data_read));

    // updates subscribtion handlers
    // _server->onUpdatesEnabled(as_cb(&Self::when_update_enabled));
    // _server->onUpdatesDisabled(as_cb(&Self::when_update_disabled));
    // _server->onConfirmationReceived(as_cb(&Self::when_confirmation_received));

    // print the handles
    SEGGER_RTT_printf(0, "AlertNotificationService service registered\r\n");
    SEGGER_RTT_printf(0, "service handle: %u\r\n", _notification_service.getHandle());
    // SEGGER_RTT_printf(0, "\thour characteristic value handle %u\r\n", _hour_char.getValueHandle());
    // SEGGER_RTT_printf(0, "\tminute characteristic value handle %u\r\n", _minute_char.getValueHandle());
    // SEGGER_RTT_printf(0, "\tsecond characteristic value handle %u\r\n", _second_char.getValueHandle());
}

void AlertNotificationService::when_data_written(const GattWriteCallbackParams *e)
{
    SEGGER_RTT_printf(0, "AlertNotificationService::when_data_written\r\n");
    SEGGER_RTT_printf(0, "\tconnection handle: %u\r\n", e->connHandle);
    SEGGER_RTT_printf(0, "\tattribute handle: %u\n", e->handle);

    if (e->handle == _answerCharacteristic.getValueHandle())
    {
        constexpr unsigned int stringTerminatorSize = 1; // end of string '\0'
        constexpr unsigned int headerSize = 3;

        const auto maxMessageSize{NotificationManager::MaximumMessageSize()};
        const auto maxBufferSize{maxMessageSize + headerSize};

        SEGGER_RTT_printf(0, "\tmaxBufferSize: %d\n", maxMessageSize + headerSize);

        // BLE_DateTime result;
        // memcpy(&result, (void *)e->data, (e->len > sizeof(BLE_DateTime) ? sizeof(BLE_DateTime): e->len));

        const auto dbgPacketLen = e->len;
        SEGGER_RTT_printf(0, "\tdbgPacketLen: %d\n", dbgPacketLen);

        unsigned int bufferSize = std::min(dbgPacketLen + stringTerminatorSize, maxBufferSize);
        auto messageSize = std::min(maxMessageSize, (bufferSize-headerSize));

        SEGGER_RTT_printf(0, "\tbufferSize: %d\n", bufferSize);
        SEGGER_RTT_printf(0, "\tmessageSize: %d\n", messageSize);

        Categories category;
        NotificationManager::Notification notif;

        memcpy(notif.message.data(), (void *)&e->data[headerSize], messageSize - 1);
        memcpy(&category, (void *)e->data, 1);

        // os_mbuf_copydata(ctxt->om, headerSize, messageSize-1, notif.message.data());
        // os_mbuf_copydata(ctxt->om, 0, 1, &category);
        notif.message[messageSize - 1] = '\0';

        for (size_t i = 0; i < notif.message.size(); ++i) {
            SEGGER_RTT_printf(0, "%02X,", notif.message[i]);
        }

        SEGGER_RTT_printf(0, "\r\n");
        SEGGER_RTT_printf(0, "\tcategory: %d\r\n", category);

        // TODO convert all ANS categories to NotificationController categories
        switch(category) {
        case Categories::Call:
            SEGGER_RTT_printf(0, "\tIncomingCall\r\n");
            notif.category = NotificationManager::Categories::IncomingCall;
            break;
        default:
            SEGGER_RTT_printf(0, "\tSimpleAlert\r\n");
            notif.category = NotificationManager::Categories::SimpleAlert;
            break;
        }

        // auto event = Pinetime::System::SystemTask::Messages::OnNewNotification;
        // notificationManager.Push(std::move(notif));
        // systemTask.PushMessage(event);

        SEGGER_RTT_printf(0, "\twrite operation: %u\r\n", e->writeOp);
        SEGGER_RTT_printf(0, "\toffset: %u\r\n", e->offset);
        SEGGER_RTT_printf(0, "\tlength: %u\r\n", e->len);
        SEGGER_RTT_printf(0, "\t data: ");

        for (size_t i = 0; i < e->len; ++i) {
            SEGGER_RTT_printf(0, "%02X,", e->data[i]);
        }

        SEGGER_RTT_printf(0, "\r\n");
    }
}

// int AlertNotificationService::OnAlert(uint16_t conn_handle, uint16_t attr_handle,
//                                                     struct ble_gatt_access_ctxt *ctxt) {
//   if (ctxt->op == BLE_GATT_ACCESS_OP_WRITE_CHR) {
//     constexpr size_t stringTerminatorSize = 1; // end of string '\0'
//     constexpr size_t headerSize = 3;
//     const auto maxMessageSize {NotificationManager::MaximumMessageSize()};
//     const auto maxBufferSize{maxMessageSize + headerSize};

//     const auto dbgPacketLen = OS_MBUF_PKTLEN(ctxt->om);
//     size_t bufferSize = std::min(dbgPacketLen + stringTerminatorSize, maxBufferSize);
//     auto messageSize = std::min(maxMessageSize, (bufferSize-headerSize));
//     Categories category;

//     NotificationManager::Notification notif;
//     os_mbuf_copydata(ctxt->om, headerSize, messageSize-1, notif.message.data());
//     os_mbuf_copydata(ctxt->om, 0, 1, &category);
//     notif.message[messageSize-1] = '\0';

//     // TODO convert all ANS categories to NotificationController categories
//     switch(category) {
//       case Categories::Call:
//         notif.category = Pinetime::Controllers::NotificationManager::Categories::IncomingCall;
//         break;
//       default:
//         notif.category = Pinetime::Controllers::NotificationManager::Categories::SimpleAlert;
//         break;
//     }

//     auto event = Pinetime::System::SystemTask::Messages::OnNewNotification;
//     notificationManager.Push(std::move(notif));
//     systemTask.PushMessage(event);
//   }
//   return 0;
// }

void AlertNotificationService::AcceptIncomingCall() {
//   auto response = IncomingCallResponses::Answer;
//   auto *om = ble_hs_mbuf_from_flat(&response, 1);

//   uint16_t connectionHandle = systemTask.nimble().connHandle();

//   if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
//     return;
//   }

//   ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}

void AlertNotificationService::RejectIncomingCall() {
//   auto response = IncomingCallResponses::Reject;
//   auto *om = ble_hs_mbuf_from_flat(&response, 1);

//   uint16_t connectionHandle = systemTask.nimble().connHandle();

//   if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
//     return;
//   }

//   ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}

void AlertNotificationService::MuteIncomingCall() {
//   auto response = IncomingCallResponses::Mute;
//   auto *om = ble_hs_mbuf_from_flat(&response, 1);

//   uint16_t connectionHandle = systemTask.nimble().connHandle();

//   if (connectionHandle == 0 || connectionHandle == BLE_HS_CONN_HANDLE_NONE) {
//     return;
//   }

//   ble_gattc_notify_custom(connectionHandle, eventHandle, om);
}
