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

#ifndef __ALERT_NOTIFCATION_SERVICE_H__
#define __ALERT_NOTIFCATION_SERVICE_H__

#include "mbed.h"
#include "events/EventQueue.h"
#include "ble/GattServer.h"
#include "ble/BLE.h"
#include "CurrentTimeService.h"
#include "NotificationManager.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

#define NOTIFICATION_EVENT_SERVICE_UUID_BASE "00020001-78fc-48fe-8e23-433b3a1942d0"

namespace Mytime {

  namespace System {
    class SystemTask;
  }
  namespace Controllers {
    // class NotificationManager;

    class AlertNotificationService {
            typedef AlertNotificationService Self;
      public:
        AlertNotificationService(Mytime::Controllers::NotificationManager &notificationManager) :
            _answerCharacteristic(UUID(_answerCharUuid), nullptr, 0, NotificationManager::MessageSize, GattCharacteristic::BLE_GATT_CHAR_PROPERTIES_WRITE),
            _notificationEventCharacteristic(UUID(NOTIFICATION_EVENT_SERVICE_UUID_BASE), 0),
            _charsTable(),
            _notification_service(
                /* uuid */              UUID(_eventServiceId),
                /* characteristics */   _charsTable,
                /* numCharacteristics */ sizeof(_charsTable) / sizeof(GattCharacteristic*)),
            _server(NULL),
            _event_queue(NULL),
            _notificationManager(notificationManager)
        {
            _charsTable[0] = {&_answerCharacteristic};
            _charsTable[1] = {&_notificationEventCharacteristic};
        }
        void Init();

        void start(BLE &ble_interface, events::EventQueue &event_queue);

        /**
         * Handler called after an attribute has been written.
         */
        void when_data_written(const GattWriteCallbackParams *e);

        // int OnAlert(uint16_t conn_handle, uint16_t attr_handle,
        //                             struct ble_gatt_access_ctxt *ctxt);

        void AcceptIncomingCall();
        void RejectIncomingCall();
        void MuteIncomingCall();

        enum class IncomingCallResponses : uint8_t {
          Reject = 0x00,
          Answer = 0x01,
          Mute = 0x02
        };

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

        enum class Categories : uint8_t {
          SimpleAlert = 0x00,
          Email = 0x01,
          News = 0x02,
          Call = 0x03,
          MissedCall = 0x04,
          MmsSms = 0x05,
          VoiceMail = 0x06,
          Schedule = 0x07,
          HighPrioritizedAlert = 0x08,
          InstantMessage = 0x09,
          All = 0xff
        };

        static constexpr UUID::ShortUUIDBytes_t _answerCharUuid {0x2a46};
        static constexpr UUID::ShortUUIDBytes_t _eventServiceId {0x1811};
        
        uint16_t eventHandle;

        GattCharacteristic _answerCharacteristic;
        NotifyCharacteristic<uint16_t> _notificationEventCharacteristic;

        UUID _ct_uuid;
        UUID _ct_ans_uuid;

        GattCharacteristic *_charsTable[2];
        GattService _notification_service;
        GattServer* _server;

        events::EventQueue *_event_queue;

        NotificationManager &_notificationManager;

        uint16_t _eventHandle;
    };
  }
}

#endif //__ALERT_NOTIFCATION_SERVICE_H__