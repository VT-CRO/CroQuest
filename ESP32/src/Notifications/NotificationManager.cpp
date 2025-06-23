#include "NotificationManager.hpp"

bool hasPendingNotification = false;
String pendingNotificationMessage = "";
uint32_t pendingNotificationDuration = 2000;

void triggerNotification(const String &msg, uint32_t duration) {
  hasPendingNotification = true;
  pendingNotificationMessage = msg;
  pendingNotificationDuration = duration;
}

void processPendingNotification() {
  if (hasPendingNotification) {
    notification.show(pendingNotificationMessage, pendingNotificationDuration);
    hasPendingNotification = false;
  }
}