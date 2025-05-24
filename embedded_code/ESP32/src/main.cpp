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

// Include Games
#include "Games/simon/Simon.hpp"
#include "Games/tic_tac_toe/TicTacToe.hpp"

GameMenu menu(&tft);
AppState currentMenuState = STATE_MENU; // Start in menu for now

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {

  // Starts Boot + Speaker
  initBoot(); // Initializes SD + TFT

<<<<<<< HEAD
=======
  //Loads settings
  loadFromSettingsFile("/settings.bin");

  // initBacklightPWM(); // Set up PWM on backlight pin
  // applyBrightness(settings.brightness); // Apply saved brightness level

>>>>>>> 3c45252eaf124a5b220f3978e9edc171545efc09
  // fills screen so sound doesn't start w/ white background
  tft.fillScreen(TFT_BLACK);

  speaker(); // Speaker Start UP

  initializeBluetoothIdentifiers(); // Generate BLE name + UUIDs based on MAC

  randomSeed(esp_random());

  showBootWithLoading("/boot/assets/boot.jpg"); // Show Splash + animation
  menu.draw();                                  // Draw menu page
}

// ####################################################################################################
//  Loop
// ####################################################################################################
void loop() {
  updateAllButtons();
  menu.handleInput();
}
