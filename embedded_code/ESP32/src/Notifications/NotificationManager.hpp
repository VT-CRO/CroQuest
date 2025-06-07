// NotificationManager.hpp
#pragma once

#include "Notification.hpp"

extern bool hasPendingNotification;
extern String pendingNotificationMessage;
extern uint32_t pendingNotificationDuration;

void triggerNotification(const String &msg, uint32_t duration = 2000);
void processPendingNotification();
