// GameSetup.cpp

#include "GameSetup.hpp"
#include <Arduino.h>

// Include games headers here
// #include "Games/Tetris/Tetris.hpp"
// #include "Games/Breakout/Breakout.hpp"
// #include "Games/UNO/UNO.hpp"

void launchGameByName(const char *name) {
  Serial.print("Launching game: ");
  Serial.println(name);

  if (strcmp(name, "Tetris") == 0) {
    // launchTetris();
  } else if (strcmp(name, "Breakout") == 0) {
    // launchBreakout();
  } else if (strcmp(name, "UNO") == 0) {
    // launchUNO();
  } else {
    Serial.println("Unknown game name.");
  }
}
