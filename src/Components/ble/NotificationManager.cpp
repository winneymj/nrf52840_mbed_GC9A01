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
  notif.id = GetNextId();
  notif.valid = true;
  notifications[writeIndex] = std::move(notif);
  writeIndex = (writeIndex + 1 < TotalNbNotifications) ? writeIndex + 1 : 0;
  if(!empty)
    readIndex = (readIndex + 1 < TotalNbNotifications) ? readIndex + 1 : 0;
  else empty = false;

  newNotification = true;
}

NotificationManager::Notification NotificationManager::GetLastNotification() {
  NotificationManager::Notification notification = notifications[readIndex];
  notification.index = 1;
  return notification;
}

NotificationManager::Notification::Id NotificationManager::GetNextId() {
  return nextId++;
}

NotificationManager::Notification NotificationManager::GetNext(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if(currentIterator == (notifications.end()-1))
    result = *(notifications.begin());
  else
    result = *(currentIterator+1);

  if(result.id <= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

NotificationManager::Notification NotificationManager::GetPrevious(NotificationManager::Notification::Id id) {
  auto currentIterator = std::find_if(notifications.begin(), notifications.end(), [id](const Notification& n){return n.valid && n.id == id;});
  if(currentIterator == notifications.end() || currentIterator->id != id) return Notification{};

  auto& lastNotification = notifications[readIndex];

  NotificationManager::Notification result;

  if(currentIterator == notifications.begin())
    result = *(notifications.end()-1);
  else
    result = *(currentIterator-1);

  if(result.id >= id) return {};

  result.index = (lastNotification.id - result.id)+1;
  return result;
}

bool NotificationManager::AreNewNotificationsAvailable() {
  return newNotification;
}

bool NotificationManager::IsVibrationEnabled() {
  return vibrationEnabled;
}

void NotificationManager::ToggleVibrations() {
  vibrationEnabled = !vibrationEnabled;  
}

bool NotificationManager::ClearNewNotificationFlag() {
//   return newNotification.exchange(false);
  return newNotification;
}

size_t NotificationManager::NbNotifications() const {
  return std::count_if(notifications.begin(), notifications.end(), [](const Notification& n){ return n.valid;});
}

