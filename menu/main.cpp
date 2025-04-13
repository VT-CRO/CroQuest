// main.cpp

#include "display_setup.hpp"
#include "gfx_utils.hpp"
#include "input_utils.hpp"

LGFX tft;

void setup() {
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);

  drawBox(30, 30, 100, 50, TFT_RED);
  drawBorder(30, 30, 100, 50, TFT_WHITE);
  drawText("Hello Menu!", 35, 45, TFT_WHITE);
}

void loop() {
  // Can later use readJoystickDirection() to move UI elements
}

