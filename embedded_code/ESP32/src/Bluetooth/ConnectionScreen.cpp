// /src/Bluetooth/ConnectionScreen.cpp

#include "ConnectionScreen.hpp"

static TFT_eSPI *screen = nullptr;

void ConnectionScreen::init(TFT_eSPI &display) { screen = &display; }

void ConnectionScreen::showMessage(const String &msg) {
  if (!screen)
    return;

  screen->fillScreen(TFT_BLACK);
  screen->setTextColor(TFT_WHITE, TFT_BLACK);
  screen->setTextSize(2);
  screen->setCursor(10, 140);
  screen->println(msg);
}
