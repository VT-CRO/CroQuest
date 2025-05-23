// main.cpp

#include <Arduino.h>

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

  // initBacklightPWM(); // Set up PWM on backlight pin
  // applyBrightness(settings.brightness); // Apply saved brightness level

  // fills screen so sound doesn't start w/ white background
  tft.fillScreen(TFT_BLACK);

  speaker(); // Speaker Start UP

  initializeBluetoothIdentifiers(); // Generate BLE name + UUIDs based on MAC

  randomSeed(analogRead(0)); // Initialize random seed (happens once)
  delay(100);

  showBootWithLoading("/boot/assets/boot.jpg"); // Show Splash + animation
  menu.draw();                                  // Draw menu page
}

// ####################################################################################################
//  Loop
// ####################################################################################################
void loop() { menu.handleInput(); }
