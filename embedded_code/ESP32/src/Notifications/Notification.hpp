// src/Core/Notification.hpp
#pragma once

#include "Core/JpegDrawing.hpp"

class Notification {
public:
  Notification(TFT_eSPI &display);

  void show(const String &msg, uint32_t durationMs = 2000);
  void update(); // Call this regularly from your loop

private:
  TFT_eSPI &tft;
  TFT_eSprite bg;

  String message;
  uint32_t startTime;
  uint32_t duration;
  bool visible;

  int boxX, boxY;
  int boxW, boxH;
};

// ===== Accessible Globally =====
extern Notification notification;
