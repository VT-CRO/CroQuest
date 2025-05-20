// /src/NumPad/NumPad.cpp

#include "NumPad.hpp"
#include "BackButton/BackButton.hpp"

const int globalYOffset =
    30; // or however far down you want to shift the entire pad

int x = (480 - 170) / 2 + 1; // Based on your earlier offset
int y = (320 - 172) / 2 + 4 + globalYOffset;

NumPad::NumPad(void (*backScreen)(), void (*forwardScreen)(), 
              State * gameState, State prevState, State nextState)
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
      buttonPos[row][col].x = x + col * (buttonWidth + spacingX);
      buttonPos[row][col].y = y + row * (buttonHeight + spacingY);
    }
  }

  // // Optionally preload the background
  // drawing.drawSdJpeg("/numpad/numpad.jpg", x, y);
  drawing.pushSprite(false, true);
}

// modify button position and state, also redraws the button
void NumPad::modButtonState(enum direction direction, enum button_state state) {

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

void NumPad::drawAllButtons() {
  JpegDrawing::ImageInfo dim = drawing.getJpegDimensions("/numpad/numpad.jpg");
  int x = (480 - dim.width) / 2;
  int y = (320 - dim.height) / 2 + globalYOffset;

  drawing.drawSdJpeg("/numpad/numpad.jpg", x, y);
  drawing.pushSprite(false, true);
  back(selected, TFT_BLACK);
}

// Draw a button
void NumPad::drawButton(enum button_state state, int row_button,
                        int column_button) {
  const Position &position = buttonPos[row_button][column_button];

  float xOffset = 0;
  float yOffset = 0;

  // switch (state) {
  // case PRESSED:
  //   xOffset = 2; // Adjust to center it like BASIC
  //   yOffset = 1;
  //   break;
  // case SELECTED:
  //   xOffset = -1;
  //   yOffset = -2; // Was slightly lower than BASIC
  //   break;
  // case BASIC:
  //   xOffset = 2.5;
  //   yOffset = 0;
  //   break;
  // }

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
  drawing.drawSdJpeg(file.c_str(), position.x + xOffset, position.y + yOffset);
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
void NumPad::drawCode() {
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

void NumPad::handleButtonInput(unsigned long *lastMoveTime,
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

void NumPad::numPadSetup() {
  tft.fillScreen(TFT_BLACK);
  drawAllButtons();
  modButtonState(NumPad::NONE, NumPad::SELECTED);
}