// Menu/GameSetup.cpp

#include "GameSetup.hpp"
#include <Arduino.h>

// Include Games
#include "Games/breakout/Breakout.hpp"
#include "Games/chess/Chess.hpp"
#include "Games/connect4/Connect4.hpp"
#include "Games/memory/Memory.hpp"
#include "Games/pong/Pong.hpp"
#include "Games/simon/Simon.hpp"
#include "Games/snake/Snake.hpp"
#include "Games/tetris/Tetris.hpp"
#include "Games/tic_tac_toe/TicTacToe.hpp"

void launchGameByName(const char *name) {
  Serial.print("Launching game: ");
  Serial.println(name);

  if (strcmp(name, "Snake") == 0) {
    runSnake();
  } else if (strcmp(name, "Pong") == 0) {
    runPong();
  } else if (strcmp(name, "Tic Tac Toe") == 0) {
    runTicTacToe();
  } else if (strcmp(name, "Simon") == 0) {
    runSimon();
  } else if (strcmp(name, "Connect 4") == 0) {
    runConnect4();
  } else if (strcmp(name, "Breakout") == 0) {
    runBreakout();
  } else if (strcmp(name, "Memory") == 0) {
    runMemory();
  } else if (strcmp(name, "Tetris") == 0) {
    runTetris();
  } else if (strcmp(name, "Chess") == 0) {
    runChess();
  } else if (strcmp(name, "Checkers") == 0) {
    // runCheckers();
  } else if (strcmp(name, "UNO") == 0) {
    // runUNO();
  } else {
    Serial.println("Unknown game name.");
  }
}
