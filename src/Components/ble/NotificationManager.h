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

#ifndef __ALERT_NOTIFCATION_MANAGER_H__
#define __ALERT_NOTIFCATION_MANAGER_H__

#include "mbed.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

namespace Mytime {
  namespace Controllers {
    class NotificationManager {
      public:
        enum class Categories {Unknown, SimpleAlert, Email, News, IncomingCall, MissedCall, Sms, VoiceMail, Schedule, HighProriotyAlert, InstantMessage };
        static constexpr uint8_t MessageSize{100};

        struct Notification {
          using Id = uint8_t;
          Id id;
          bool valid = false;
          uint8_t index;
          std::array<char, MessageSize+1> message;
          Categories category = Categories::Unknown;
        };
        Notification::Id nextId {0};

      void Push(Notification&& notif);
      Notification GetLastNotification();
      Notification GetNext(Notification::Id id);
      Notification GetPrevious(Notification::Id id);
      bool ClearNewNotificationFlag();
      bool AreNewNotificationsAvailable();
      bool IsVibrationEnabled();
      void ToggleVibrations();

      static constexpr size_t MaximumMessageSize() { return MessageSize; };
      size_t NbNotifications() const;

      private:
        Notification::Id GetNextId();
        static constexpr uint8_t TotalNbNotifications = 5;
        std::array<Notification, TotalNbNotifications> notifications;
        uint8_t readIndex = 0;
        uint8_t writeIndex = 0;
        bool empty = true;
        // std::atomic<bool> newNotification{false};
        bool newNotification{false};
        bool vibrationEnabled = true;
    };
  }
}

#endif// __ALERT_NOTIFCATION_MANAGER_H__
