// gfx_utils.hpp

#pragma once
#include <cstdint>
#include "display_setup.hpp"

void drawBox(int x, int y, int w, int h, uint16_t color);

void drawBorder(int x, int y, int w, int h, uint16_t borderColor);

void drawText(const char* text, int x, int y, uint16_t color);

void drawBitmapImage(int x, int y, const uint16_t* image, int w, int h);
