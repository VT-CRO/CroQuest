// src/EndScreen/EndScreen.hpp

#pragma once

#include "BackButton/BackButton.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"

#include <TFT_eSPI.h>
#include <vector>

class EndScreen {
public:
  enum Selection { BACK_BUTTON = 0, RESTART_GAME = 1, GAME_MENU = 2 };

  // ========== Handle User Input ========== //
  bool handleUserInput();

  bool exit; // If true exit to menu

  // Constructor
  EndScreen(std::vector<String> &names, std::vector<int> &scores,
            bool multiplayer, char *playerName, int score)
      : playerNames(names), playerScores(scores), multiplayer(multiplayer),
        playerName(playerName), score(score) {
    exit = false;
  }

private:
  enum State { SCORE_BOARD, ENDSCREEN };
  State currentState = SCORE_BOARD;
  Selection currentSelection = RESTART_GAME;

  bool multiplayer;
  std::vector<String> &playerNames;
  std::vector<int> &playerScores;
  char *playerName;
  int score;

  // ========== Draw Seleciton Buttons ========== //
  void drawingSelections(uint16_t bgcolor);

  // ========== Score Board ========== //
  void scoreBoardScreen();

  // ========== End Screen ========== //
  void gameOverScreen();
};