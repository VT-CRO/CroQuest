// main.cpp

#include <Arduino.h>

#include "Boot/Boot.hpp"
#include "Core/AppState.hpp"
#include "Core/Buttons.hpp"
#include "Menu/GameMenu.hpp"

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
  speaker();  // Speaker Start UP
  initBoot(); // Initializes SD + TFT

  randomSeed(analogRead(0)); // Initialize random seed (happens once)
  delay(100);

  showBootWithLoading("/boot/assets/boot.jpg"); // Show Splash + animation
  menu.draw();                                  // Draw menu page
}

// ####################################################################################################
//  Loop
// ####################################################################################################
void loop() {

  // Keep checking for Input
  // updateAllButtons();
  menu.handleInput();
}
