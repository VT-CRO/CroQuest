// src/Settings/Settings.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Menu/GameMenu.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/BrightnessMenu/Brightness.hpp"

#include <TFT_eSPI.h>

struct SettingsData {
  bool soundOn = true;
  int brightness = 5; // Default brightness level
};
extern SettingsData settings;

void initSettings(TFT_eSPI *display);
void runSettings();
