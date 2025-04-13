// gfx_utils.cpp
#include "gfx_utils.hpp"

void drawBox(int x, int y, int w, int h, uint16_t color) {
  tft.fillRect(x, y, w, h, color);
}

void drawBorder(int x, int y, int w, int h, uint16_t borderColor) {
  tft.drawRect(x, y, w, h, borderColor);
}

void drawText(const char* text, int x, int y, uint16_t color) {
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.print(text);
}

void drawBitmapImage(int x, int y, const uint16_t* image, int w, int h) {
  tft.pushImage(x, y, w, h, image);
}

