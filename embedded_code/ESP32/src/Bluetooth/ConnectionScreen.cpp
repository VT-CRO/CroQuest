// ConnectionScreen.cpp
#include "ConnectionScreen.hpp"
#include <TFT_eSPI.h>

extern TFT_eSPI tft;

void ConnectionScreen::showMessage(const String &msg) {
  tft.fillScreen(TFT_BLACK);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.setTextSize(2);
  tft.setCursor(10, 140);
  tft.println(msg);
}
