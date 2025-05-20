// GameSetup.cpp

#include "GameSetup.hpp"
#include <Arduino.h>

// Include Games
#include "Games/simon/Simon.hpp"
#include "Games/tic_tac_toe/TicTacToe.hpp"

// Include games headers here
// #include "Games/Tetris/Tetris.hpp"
// #include "Games/Breakout/Breakout.hpp"
// #include "Games/UNO/UNO.hpp"

void launchGameByName(const char *name) {
  Serial.print("Launching game: ");
  Serial.println(name);

  if (strcmp(name, "Snake") == 0) {
    // launchTetris();
  } else if (strcmp(name, "Pong") == 0) {
    // launchBreakout();
  } else if (strcmp(name, "Tic Tac Toe") == 0) {
    runTicTacToe();
  } else if (strcmp(name, "Simon") == 0) {
    runSimon();
  } else if (strcmp(name, "Connect 4") == 0) {
    // launchConnect4();
  } else if (strcmp(name, "Breakout") == 0) {
    // launchBreakout();
  } else if (strcmp(name, "Memory") == 0) {
    // launchMemory();
  } else if (strcmp(name, "Tetris") == 0) {
    // launchTetris();
  } else if (strcmp(name, "Chess") == 0) {
    // launchChess();
  } else if (strcmp(name, "Checkers") == 0) {
    // launchCheckers();
  } else if (strcmp(name, "UNO") == 0) {
    // launchUNO();
  } else {
    Serial.println("Unknown game name.");
  }
}
