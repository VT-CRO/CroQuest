// main.cpp

#include <Arduino.h>
#include <esp_system.h>

#include "Bluetooth/BluetoothCommon.hpp"
#include "Boot/Boot.hpp"
#include "Core/AppState.hpp"
#include "Core/BrightnessControl.hpp"
#include "Core/Buttons.hpp"
#include "Menu/GameMenu.hpp"
#include "SettingsMenu/Settings/Settings.hpp" // for `settings.brightness`

GameMenu menu(&tft);                    // Create a GameMenu instance
AppState currentMenuState = STATE_MENU; // Start in menu for now

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {

  // Initialize SD + TFT
  initBoot();

  // Loads settings
  loadFromSettingsFile("/settings.bin");

  // Speaker Start UP
  speaker();

  // fills screen so sound doesn't start w/ white background
  tft.fillScreen(TFT_BLACK);

  // Generate BLE name + UUIDs
  initializeBluetoothIdentifiers();

  // Initialize Bluetooth Peripheral
  randomSeed(esp_random());

  // Initialize Boot Screen
  showBootWithLoading("/boot/assets/boot.jpg");

  // Draw Menu Page
  menu.draw();
}

// ####################################################################################################
//  Loop
// ####################################################################################################
void loop() {
  updateAllButtons();
  menu.handleInput();
}
