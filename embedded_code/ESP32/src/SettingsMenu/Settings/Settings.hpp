// src/Settings/Settings.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Menu/GameMenu.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/NameMenu/Name.hpp"

// WILL BRING THIS BACK IF WE FIGURE OUT A WAY TO CHANGE BRIGHTNESS
// #include "SettingsMenu/BrightnessMenu/Brightness.hpp"

#include <TFT_eSPI.h>

struct SettingsData {
  int volume = 100; // Percentage
  // int brightness = 5; // Default brightness level
  char name[6] = "CROWS";
};
extern SettingsData settings;

// void initSettings(TFT_eSPI *display);
void runSettings();

//Background color for the setting menus
#define SETTINGS_BG_COLOR 0x528A

// ============== API ================ //
bool loadFromSettingsFile(const char* path);

