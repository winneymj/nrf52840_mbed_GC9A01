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

#include "mbed.h"
#include "NotificationManager.h"

extern "C"{
  #include "SEGGER_RTT.h"
}

using namespace Mytime::Controllers;

constexpr uint8_t NotificationManager::MessageSize;

void NotificationManager::Push(NotificationManager::Notification &&notif) {
    SEGGER_RTT_printf(0, "NotificationManager::Push: START\r\n");

    notif.id = GetNextId();
    notif.valid = true;
    _notifications[_writeIndex] = std::move(notif);
    _writeIndex = (_writeIndex + 1 < TotalNbNotifications) ? _writeIndex + 1 : 0;
    if(!_empty)
        _readIndex = (_readIndex + 1 < TotalNbNotifications) ? _readIndex + 1 : 0;
    else _empty = false;

    SEGGER_RTT_printf(0, "\tnotif.id: %u, valid: %u, index: %u, _writeIndex: %u, _readIndex: %u, notif.category: %u\r\n", notif.id,
        notif.valid,
        notif.index,
        _writeIndex,
        _readIndex,
        notif.category);

    _newNotification = true;
    SEGGER_RTT_printf(0, "NotificationManager::Push: END\r\n");
}

NotificationManager::Notification NotificationManager::GetLastNotification() {
  NotificationManager::Notification notification = _notifications[_readIndex];
  notification.index = 1;
  return notification;
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(_notifications.begin(), _notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == _notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = _notifications[_readIndex];

  NotificationManager::Notification result;

  if(currentIterator == (_notifications.end()-1))
    result = *(_notifications.begin());
  else
    result = *(currentIterator+1);

  if(result.id <= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(_notifications.begin(), _notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == _notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = _notifications[_readIndex];

  NotificationManager::Notification result;

  if(currentIterator == _notifications.begin())
    result = *(_notifications.end()-1);
  else
    result = *(currentIterator-1);

  if(result.id >= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

bool NotificationManager::AreNewNotificationsAvailable() {
  return _newNotification;
}

bool NotificationManager::IsVibrationEnabled() {
  return _vibrationEnabled;
}

void NotificationManager::ToggleVibrations() {
  _vibrationEnabled = !_vibrationEnabled;  
}

bool NotificationManager::ClearNewNotificationFlag() {
//   return _newNotification.exchange(false);
  return _newNotification;
}

size_t NotificationManager::NbNotifications() const {
  return std::count_if(_notifications.begin(), _notifications.end(), [](const Notification& n){ return n.valid;});
}

