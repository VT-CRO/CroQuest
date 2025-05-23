#pragma once

// #include "Boot/Boot.hpp"
#include "Core/BrightnessControl.hpp"
#include "Core/Buttons.hpp"
#include "SettingsMenu/Settings/Settings.hpp"
#include <Arduino.h>

#include <TFT_eSPI.h>

void initBrightness(TFT_eSPI *display);
void runBrightnessMenu();
void fakeDimOverlay(int brightnessLevel);
