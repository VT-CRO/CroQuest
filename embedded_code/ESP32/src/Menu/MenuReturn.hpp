// Menu/MenuReturn.hpp

#pragma once
#include "Core/Buttons.hpp"
#include <TFT_eSPI.h>

// Flag to track exit request
static bool shouldExitToMenu = false;

// Internal debounce timer
inline bool checkStartButtonAndExit(TFT_eSPI &tft) {
  static unsigned long lastPressTime = 0;
  const unsigned long debounceDelay = 300;

  if (millis() - lastPressTime < debounceDelay)
    return false;

  if (Start.wasJustPressed()) {
    lastPressTime = millis();

    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("Returning to Menu...", tft.width() / 2, tft.height() / 2);
    delay(400); // Optional UX delay

    shouldExitToMenu = true;
    return true;
  }
  return false;
}

inline void resetExitFlag() { shouldExitToMenu = false; }

inline bool getExitFlag() { return shouldExitToMenu; }
