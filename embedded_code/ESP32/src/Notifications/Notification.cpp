// src/Core/Notification.cpp
#include "Notification.hpp"

extern TFT_eSPI tft;

// ======= Implementation =======
Notification::Notification(TFT_eSPI &display)
    : tft(display), bg(&display), visible(false) {}

void Notification::show(const String &msg, uint32_t durationMs) {
  message = msg;
  duration = durationMs;
  startTime = millis();
  visible = true;

  // Compute centered position
  int shadowOffset = 3;
  boxW = 260;
  boxH = 40;
  boxY = 27;
  boxX = (tft.width() - boxW) / 2;

  // Allocate sprite to include shadow area
  int spriteW = boxW + shadowOffset;
  int spriteH = boxH + shadowOffset;

  bg.setColorDepth(16);
  bg.createSprite(spriteW, spriteH);

  // Save the background including shadow area
  uint16_t *buffer = (uint16_t *)malloc(spriteW * spriteH * sizeof(uint16_t));
  if (buffer) {
    tft.readRect(boxX, boxY, spriteW, spriteH, buffer);
    bg.pushImage(0, 0, spriteW, spriteH, buffer);
    free(buffer);
  }

  // === Draw drop shadow ===
  tft.fillRoundRect(boxX + shadowOffset, boxY + shadowOffset, boxW, boxH, 6,
                    tft.color565(40, 40, 40));

  // === Draw main box ===
  uint16_t bgColor = tft.color565(30, 30, 30);
  tft.fillRoundRect(boxX, boxY, boxW, boxH, 6, bgColor);
  tft.drawRoundRect(boxX, boxY, boxW, boxH, 6, TFT_WHITE);

  // === Draw text ===
  tft.setTextColor(TFT_GREEN, bgColor);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(message, boxX + boxW / 2, boxY + boxH / 2);

  delay(3600);
}

void Notification::update() {
  if (visible && millis() - startTime > duration) {
    visible = false;
    // Restore the saved background
    bg.pushSprite(boxX, boxY);
    bg.deleteSprite(); // Clean up memory
  }
}

Notification notification(tft);
