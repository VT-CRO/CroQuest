// /src/NumPad/NumPad.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include <TFT_eSPI.h>
#include <string>
#include "BackButton/BackButton.hpp"

extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, up, down, left, right;

//Similar to Java's generic classes
template<typename EnumType>
class NumPad {
public:
  enum direction { UP, DOWN, LEFT, RIGHT, NONE };
  enum button_state { PRESSED, BASIC, SELECTED };
  enum button_type { DEL = 10, ENTER = 11 };

  // Constructor
  NumPad(void (*backScreen)(), void (*forwardScreen)(), 
          EnumType * gameState, EnumType prevState, EnumType nextState);

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

  //pointer to the gameState
  EnumType * gameState;

  // The two states that the numpad can jump to
  EnumType prevState;
  EnumType nextState;

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

// ================ IMPLEMENTATION =========================

// Could only get the template to work when I placed the implementation
// in the header file instead of a .cpp file

static const int globalYOffset =
    30; // or however far down you want to shift the entire pad

static JpegDrawing::ImageInfo numpad_dim =
drawing.getJpegDimensions("/numpad/numpad.jpg");

template<typename EnumType>
NumPad<EnumType>::NumPad(void (*backScreen)(), void (*forwardScreen)(), 
              EnumType * gameState, EnumType prevState, EnumType nextState)
              : backScreen(backScreen), forwardScreen(forwardScreen){
  this->gameState = gameState;
  this->prevState = prevState;
  this->nextState = nextState;
  const int buttonWidth = 47;
  const int buttonHeight = 31;
  const int spacingX = 14;
  const int spacingY = 14;

  for (int row = 0; row < 4; row++) {
    for (int col = 0; col < 3; col++) {
      // The image is 195 x 195
      buttonPos[row][col].x = ((480 - 195) / 2 + 14) + col * (buttonWidth + spacingX);
      buttonPos[row][col].y = ((320 - 195) / 2 + 16) + globalYOffset + row * (buttonHeight + spacingY);
    }
  }

  // // Optionally preload the background
  // drawing.drawSdJpeg("/numpad/numpad.jpg", x, y);
  drawing.pushSprite(false, true);
}

// modify button position and state, also redraws the button
template<typename EnumType>
void NumPad<EnumType>::modButtonState(enum direction direction, enum button_state state) {

  int prev_row = row;
  int prev_col = column;

  switch (direction) {

  case UP:
    if (row > 0)
      row--;
    break;

  case DOWN:
    if (row < 3)
      row++;
    break;

  case LEFT:
    if (column > 0)
      column--;
    break;
    
  case RIGHT:
    if (column < 2)
      column++;
    break;

  default:
    break;
  }
  // "Erase" previous selection by redrawing just that tile from the base numpad
  // image
  if (prev_row != row || prev_col != column) {
    // === Step 1: Clean up leftover SELECTED pixels from background ===
    JpegDrawing::ImageInfo dim =
        drawing.getJpegDimensions("/numpad/numpad.jpg");
    int fullX = (480 - dim.width) / 2;
    int fullY = (320 - dim.height) / 2 + globalYOffset;

    // Match your button dimensions exactly
    int buttonW = 47;
    int buttonH = 31;

    int srcX = buttonPos[prev_row][prev_col].x - fullX;
    int srcY = buttonPos[prev_row][prev_col].y - fullY;
    int dstX = buttonPos[prev_row][prev_col].x;
    int dstY = buttonPos[prev_row][prev_col].y;

    drawing.drawJpegTile("/numpad/numpad.jpg", srcX, srcY, buttonW, buttonH,
                         dstX, dstY);
    drawing.pushSprite(false, true);

    // === Step 2: Restore full BASIC button ===
    drawButton(BASIC, prev_row, prev_col);
  }

  if (state == PRESSED) {
    if (pad[row][column] == DEL) {
      if (code.length() > 0) {
        // removes right-most number
        code.pop_back();
      }

    } else if (pad[row][column] == ENTER) {
      //////// TO-DO /////////////
      /// BLUETOOTH LOGIC //////////
      
      // Temp logic, simply changes state and draws to the screen
      (*gameState) = nextState;
      forwardScreen();
      return;
    } else if(selected == 0){
        // Back Button Selected - returns to previous screen,
        // resetting the gamestate and selected number
        (*gameState) = prevState;
        backScreen();
        selected = 1;
        return;
    }
    else {
      if (code.length() < max_length) {
        std::string button = std::to_string(pad[row][column]);
        code += button;
      }
    }
  }

  // Draws the code and button state
  drawCode();
  drawButton(state, row, column);
}

template<typename EnumType>
void NumPad<EnumType>::drawAllButtons() {
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions("/numpad/numpad.jpg");
  int x = (480 - dim.width) / 2;
  int y = (320 - dim.height) / 2 + globalYOffset;

  drawing.drawSdJpeg("/numpad/numpad.jpg", x, y);
  drawing.pushSprite(false, true);
  back(selected, TFT_BLACK);
}

// Draw a button
template<typename EnumType>
void NumPad<EnumType>::drawButton(enum button_state state, int row_button,
                        int column_button) {
  const Position &position = buttonPos[row_button][column_button];

  std::string basePath;
  switch (state) {
  case PRESSED:
    basePath = "/numpad/pressed/";
    break;
  case BASIC:
    basePath = "/numpad/basic/";
    break;
  case SELECTED:
    basePath = "/numpad/selected/";
    break;
  }

  std::string file =
      basePath + std::to_string(pad[row_button][column_button]) + ".jpg";
  drawing.drawSdJpeg(file.c_str(), position.x, position.y);
  drawing.pushSprite(false, true);

  if (state == PRESSED) {
    delay(100);
    // Return to BASIC image
    std::string fallback = "/numpad/basic/" +
                           std::to_string(pad[row_button][column_button]) +
                           ".jpg";
    drawing.drawSdJpeg(fallback.c_str(), position.x, position.y);
    drawing.pushSprite(false, true);
  }
}

// Draw Code
template<typename EnumType>
void NumPad<EnumType>::drawCode() {
  // --- Draw Label ---
  tft.setTextDatum(TL_DATUM); // Top-left corner for label
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  const char *label = "Enter a Host Code:";
  int labelX = (SCREEN_WIDTH - tft.textWidth(label)) / 2;
  int labelY = 25;
  tft.drawString(label, labelX, labelY);

  // --- Draw Code Centered Below ---
  tft.setTextDatum(MC_DATUM); // Middle-center for code
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);

  int codeY = labelY + 50; // space below the label
  int codeHeight = 8 * 4;  // estimated height of text at size 4

  // Clear code area
  tft.fillRect(0, codeY - codeHeight / 2, SCREEN_WIDTH, codeHeight, TFT_BLACK);

  // Draw centered code
  tft.drawString(code.c_str(), SCREEN_WIDTH / 2, codeY);
}

template<typename EnumType>
void NumPad<EnumType>::handleButtonInput(unsigned long *lastMoveTime,
                               const long moveDelay) {
  if (millis() - *lastMoveTime > moveDelay) {
    // Press Logic
    if (!A.isPressed() && !pressed) {
        modButtonState(NumPad::NONE, NumPad::PRESSED);
        pressed = true;
      *lastMoveTime = millis();
    } else if (A.isPressed() && pressed) {
      modButtonState(NumPad::NONE, NumPad::SELECTED);
      pressed = false;
    }
    if (up.isPressed()) {
      // Back button selection logic to
      // highlight the back button
      if(row <= 0 && selected == 1){
        selected = 0;
        modButtonState(NumPad::NONE, NumPad::BASIC);
        back(selected, TFT_BLACK);
      }else{
        modButtonState(NumPad::UP, NumPad::SELECTED);
      }
      *lastMoveTime = millis();
    } else if (down.isPressed()) {
      // Back button selection logic to 
      // de-select the button
      if(row <= 0 && selected == 0){
        selected = 1;
        back(selected, TFT_BLACK);
        modButtonState(NumPad::NONE, NumPad::SELECTED);
      }else{
        modButtonState(NumPad::DOWN, NumPad::SELECTED);
      }
      *lastMoveTime = millis();
    } else if (right.isPressed()) {
      modButtonState(NumPad::RIGHT, NumPad::SELECTED);
      *lastMoveTime = millis();
    } else if (left.isPressed()) {
      modButtonState(NumPad::LEFT, NumPad::SELECTED);
      *lastMoveTime = millis();
    }
  }
}

template<typename EnumType>
void NumPad<EnumType>::numPadSetup() {
  tft.fillScreen(TFT_BLACK);

  //Deletes any sprite that drawing may contain
  drawing.deleteSprite();
  
  drawAllButtons();
  modButtonState(NumPad::NONE, NumPad::SELECTED);
}