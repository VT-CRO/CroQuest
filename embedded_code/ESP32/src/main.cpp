// main.cpp

#include <Arduino.h>

#include "Boot/Boot.hpp"
#include "Menu/GameMenu.hpp"

GameMenu menu(&tft);

// ####################################################################################################
//  Setup
// ####################################################################################################
void setup() {

  // Starts Boot
  initBoot();                                   // Initializes SD + TFT
  showBootWithLoading("/boot/assets/boot.jpg"); // Show Splash + animation

  menu.draw(); // Draw menu page
}

// ####################################################################################################
//  Loop
// ####################################################################################################
void loop() {

  // Keep checking for Input
  menu.handleInput();
}
