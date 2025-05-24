// src/Settings/Settings.hpp

#include "Settings.hpp"

#define SETTINGS_BG_COLOR 0x528A
SettingsData settings; // Define global

// static TFT_eSPI *tft = nullptr;

//tft object
extern TFT_eSPI tft;

//Saving settings to file
static bool saveToSettingsFile(const char* path);

void runSettings() {

  const char *options[] = {"Name", "Volume", "Back"};
  const int optionCount = sizeof(options) / sizeof(options[0]);
  int selectedOption = 0;
  int lastDrawnOption = -1;

  constexpr int optionHeight = 48; // Slightly taller for spacing
  constexpr int optionGap = 6; // Space between options (where divider lives)
  constexpr int buttonPadding = 10;
  constexpr int textLeftPadding = 20;
  constexpr int titleHeight = 50;
  constexpr uint16_t dividerColor = 0x7BEF;

  // === Local function to draw a single option ===
  auto drawOption = [&](int index, bool selected) {
    int y = titleHeight + 10 + index * (optionHeight + optionGap);
    constexpr int selectorMaxRadius = MenuLayout::SELECTOR_THICKNESS;

    // Full area clear (with buffer)
    tft.fillRect(buttonPadding - selectorMaxRadius - 1,
                  y - selectorMaxRadius - 1,
                  tft.width() - 2 * (buttonPadding - selectorMaxRadius - 1),
                  optionHeight + 2 * selectorMaxRadius + 2, SETTINGS_BG_COLOR);

    // Selector
    if (selected) {
      for (int j = 0; j < MenuLayout::SELECTOR_THICKNESS; j++) {
        tft.drawRoundRect(buttonPadding - j, y - j,
                           tft.width() - 2 * buttonPadding + 2 * j,
                           optionHeight + 2 * j, 4, TFT_WHITE);
      }
    }

    // Text
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, SETTINGS_BG_COLOR);
    tft.drawString(options[index], textLeftPadding, y + 12);

    // Divider ABOVE (if not the first option)
    if (index > 0) {
      int dividerY = y - optionGap / 2;
      tft.drawLine(buttonPadding, dividerY, tft.width() - buttonPadding,
                    dividerY, dividerColor);
    }

    // Divider BELOW (if not the last option)
    if (index < optionCount - 1) {
      int dividerY = y + optionHeight + optionGap / 2;
      tft.drawLine(buttonPadding, dividerY, tft.width() - buttonPadding,
                    dividerY, dividerColor);
    }
  };

  // === Draw header title once ===
  tft.fillScreen(SETTINGS_BG_COLOR);
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor(TFT_WHITE, SETTINGS_BG_COLOR);
  tft.setTextSize(3);
  tft.drawString("SETTINGS", textLeftPadding, 10);

  // White thick divider under title
  int dividerTop = 10 + 24 + 12; // y = title_y + approx text height + padding
  for (int i = 0; i < 2; i++) {  // thickness = 2px
    tft.drawLine(buttonPadding, dividerTop + i, tft.width() - buttonPadding,
                  dividerTop + i, TFT_WHITE);
  }

  // === Initial draw of all options ===
  for (int i = 0; i < optionCount; i++) {
    drawOption(i, i == selectedOption);
  }
  lastDrawnOption = selectedOption;

  // === Input loop ===
  while (true) {
    if (down.wasJustPressed()) {
      drawOption(selectedOption, false); // clear current selector
      selectedOption = (selectedOption + 1) % optionCount;
      drawOption(selectedOption, true); // draw new selector
      lastDrawnOption = selectedOption;
      playSelectBeep();
      delay(150);
    } else if (up.wasJustPressed()) {
      drawOption(selectedOption, false);
      selectedOption = (selectedOption - 1 + optionCount) % optionCount;
      drawOption(selectedOption, true);
      lastDrawnOption = selectedOption;
      playSelectBeep();
      delay(150);
    } else if (A.wasJustPressed()) {
      if (strcmp(options[selectedOption], "Back") == 0) {
        saveToSettingsFile("/settings.bin");
        backAudio();
        break;
      } 
      // WILL BRING THIS BACK IF WE FIGURE OUT A WAY TO CHANGE BRIGHTNESS

      // else if (strcmp(options[selectedOption], "Brightness") == 0) {
      //   runBrightnessMenu();
      // } 
      else if (strcmp(options[selectedOption], "Volume") == 0) {
        playPressSound();
        runAudioMenu();
      }
      else if (strcmp(options[selectedOption], "Name") == 0){
        playPressSound();
        runNameMenu();
      }

      // === Full screen reset after returning from Brightness ===
      tft.fillScreen(SETTINGS_BG_COLOR);

      // Redraw title
      tft.setTextDatum(TL_DATUM);
      tft.setTextColor(TFT_WHITE, SETTINGS_BG_COLOR);
      tft.setTextSize(3);
      tft.drawString("Settings", textLeftPadding, 10);

      // Redraw thick white divider under title
      int dividerTop = 10 + 24 + 12;
      for (int i = 0; i < 2; i++) {
        tft.drawLine(buttonPadding, dividerTop + i,
                      tft.width() - buttonPadding, dividerTop + i, TFT_WHITE);
      }

      // Redraw all options and selector
      for (int i = 0; i < optionCount; i++) {
        drawOption(i, i == selectedOption);
      }

      delay(200);
    }

    delay(10);
  }
}

// ============= SAVING AND LOADING SETTINGS =================== //

//Doesn't need to be called anywhere else
static bool saveToSettingsFile(const char* path) {
  File file = SD.open(path, FILE_WRITE);
  if (!file) {
    Serial.println("Failed to open settings file for writing");
    return false;
  }

  file.write((uint8_t*)&settings, sizeof(SettingsData));
  file.close();
  return true;
}

//Needs to be called in bootup
bool loadFromSettingsFile(const char* path) {
  File file = SD.open(path, FILE_READ);
  if (!file) {
    Serial.println("Settings file not found, using defaults");
    return false;
  }

  if (file.read((uint8_t*)&settings, sizeof(SettingsData)) != sizeof(SettingsData)) {
    Serial.println("Failed to read complete settings");
    file.close();
    return false;
  }

  file.close();
  return true;
}

