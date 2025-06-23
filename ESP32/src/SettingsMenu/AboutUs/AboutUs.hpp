// src/SettingsMenu/AboutUs/AboutUs.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/Settings/Settings.hpp"

#include <TFT_eSPI.h>

// Run About Us
void runAboutUs();

void drawBoldString(const char *text, int x, int y, uint16_t color,
                    uint16_t bg);
