// /src/NumPad/NumPad.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include <TFT_eSPI.h>
#include <string>
#include "States/States.hpp"

extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, up, down, left, right;

class NumPad {
public:
  enum direction { UP, DOWN, LEFT, RIGHT, NONE };
  enum button_state { PRESSED, BASIC, SELECTED };
  enum button_type { DEL = 10, ENTER = 11 };

  // Constructor
  NumPad(void (*backScreen)(), void (*forwardScreen)(), 
          State * gameState, State prevState, State nextState);

  void drawAllButtons();
  void handleButtonInput(unsigned long *lastMoveTime,
                               const long moveDelay);
  void modButtonState(direction dir, button_state state);
  void numPadSetup();

  std::string getCode() const { return code; }
  void clearCode() { code.clear(); }

private:

  void (*backScreen)();
  void (*forwardScreen)();
  State * gameState;

  // The two states that the numpad can jump to
  State prevState;
  State nextState;

  const int SCREEN_WIDTH = 480;

  // Pad layout
  int pad[4][3] = {{1, 2, 3}, {4, 5, 6}, {7, 8, 9}, {DEL, 0, ENTER}};

  struct Position {
    int x;
    int y;
  };
  Position buttonPos[4][3];

  // State
  bool pressed = false;
  int row = 0;
  int column = 0;
  const int max_length = 6;
  std::string code;

  // Drawing
  int buttonWidth = 50;
  int buttonHeight = 45;

  int selected = 1;

  void drawButton(button_state state, int row, int col);
  void drawCode();
};
