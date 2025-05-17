// GameMenu.cpp

#include "GameMenu.hpp"
#include <JPEGDecoder.h>
#include <SD.h>

#include "Menu/GameSetup.hpp"

using namespace MenuLayout; // Use layout constants

// ###################### Initialize Buttons ######################
GameMenu::GameMenu(TFT_eSPI *tft)
    : tft(tft), selectedIndex(0), buttonA(22, "A", DIGITAL),
      buttonB(39, "B", DIGITAL), buttonStart(36, "Start", DIGITAL),
      buttonUp(35, "Up", ANALOG_INPUT, 2000, 3600),
      buttonRight(35, "Right", ANALOG_INPUT, 3601, 4095),
      buttonLeft(34, "Left", ANALOG_INPUT, 2000, 3600),
      buttonDown(34, "Down", ANALOG_INPUT, 3601, 4095) {

  // ###################### Lists Games ######################
  gameBoxes[0] = {"Snake", 0, 0};
  gameBoxes[1] = {"Pong", 0, 1};
  gameBoxes[2] = {"Tic Tac Toe", 0, 2};
  gameBoxes[3] = {"Simon", 0, 3};
  gameBoxes[4] = {"Connect 4", 1, 0};
  gameBoxes[5] = {"Breakout", 1, 1};
  gameBoxes[6] = {"Memory", 1, 2};
  gameBoxes[7] = {"Tetris", 1, 3};

  // TODO: Add a second page or slide menu for this games.
  // gameBoxes[8] = {"Chess", 2, 0};
  // gameBoxes[9] = {"Checkers", 2, 1};
  // gameBoxes[10] = {"UNO", 2, 2};
}

// ###################### Draw Screen ######################
void GameMenu::draw() { drawPage(); }

// ###################### Draw Menu Interface ######################
void GameMenu::drawPage() {

  // Screen Background Color (Gray)
  tft->fillScreen(BACKGROUND_COLOR);

  // Check for Background.jpg file
  File jpegFile = SD.open(MENU_BG_PATH);
  if (!jpegFile) {
    Serial.println("Menu background not found!");
    return;
  }

  // Set correct colors when drawing
  tft->setSwapBytes(true);

  // Positions the Jpeg image
  if (JpegDec.decodeSdFile(jpegFile)) {
    while (JpegDec.read()) {
      int x = JpegDec.MCUx * JpegDec.MCUWidth;
      int y = JpegDec.MCUy * JpegDec.MCUHeight;
      tft->pushImage(x, y, JpegDec.MCUWidth, JpegDec.MCUHeight, JpegDec.pImage);
    }
  }

  jpegFile.close(); // Closes image.

  // Draw all game names
  tft->setTextColor(TFT_WHITE, BACKGROUND_COLOR);
  tft->setTextSize(1);         // You can increase to 2 if needed
  tft->setTextDatum(MC_DATUM); // Middle center alignment (requires setFreeFont
                               // or use default)

  for (int i = 0; i < ITEM_COUNT; i++) {
    int row = i / ITEMS_PER_ROW;
    int col = i % ITEMS_PER_ROW;
    int x = LEFT_MARGIN + col * (ICON_SIZE + H_SPACING);
    int y = TOP_MARGIN + row * (ICON_SIZE + MARGIN_Y);

    int textX = x + ICON_SIZE / 2;
    int textY = y + ICON_SIZE + 10; // 10px below the icon

    tft->drawString(gameBoxes[i].name, textX, textY);
  }

  // Draw initial selector
  int row = selectedIndex / ITEMS_PER_ROW;
  int col = selectedIndex % ITEMS_PER_ROW;

  int x = LEFT_MARGIN + col * (ICON_SIZE + H_SPACING);
  int y = TOP_MARGIN + row * (ICON_SIZE + MARGIN_Y);

  // Draw a "thicker" rounded rectangle by layering multiple ones
  for (int i = 0; i < SELECTOR_THICKNESS; i++) {
    tft->drawRoundRect(x - i, y - i, ICON_SIZE + 2 * i, ICON_SIZE + 2 * i,
                       SELECTOR_RADIUS, TFT_WHITE);
  }
}

// ###################### Handle Input User ######################
void GameMenu::handleInput() {
  static unsigned long lastInput = 0;
  static int previousIndex = 0;
  if (millis() - lastInput < 200)
    return;

  bool moved = false;

  // Checks what button was pressed and executes movement (ANALOG)
  if (buttonUp.isPressed()) {
    if (selectedIndex >= ITEMS_PER_ROW) {
      selectedIndex -= ITEMS_PER_ROW;
      moved = true;
    }
  } else if (buttonDown.isPressed()) {
    if (selectedIndex + ITEMS_PER_ROW < ITEM_COUNT) {
      selectedIndex += ITEMS_PER_ROW;
      moved = true;
    }
  } else if (buttonLeft.isPressed()) {
    if (selectedIndex == 0) {
      selectedIndex = ITEM_COUNT - 1;
    } else {
      selectedIndex--;
    }
    moved = true;
  } else if (buttonRight.isPressed()) {
    if (selectedIndex == ITEM_COUNT - 1) {
      selectedIndex = 0;
    } else {
      int nextIndex = selectedIndex + 1;
      if (nextIndex < ITEM_COUNT) {
        selectedIndex = nextIndex;
      }
    }
    moved = true;
  }

  // Checks what button was pressed and executes movement (DIGITAL)
  if (buttonA.wasJustPressed()) {
    launchGameByName(gameBoxes[selectedIndex].name);
    lastInput = millis();
    return;
  }

  // Selector Movement
  if (moved) {
    // Clear old selector
    int prevRow = previousIndex / ITEMS_PER_ROW;
    int prevCol = previousIndex % ITEMS_PER_ROW;
    int px = LEFT_MARGIN + prevCol * (ICON_SIZE + H_SPACING);
    int py = TOP_MARGIN + prevRow * (ICON_SIZE + MARGIN_Y);

    for (int i = 0; i < SELECTOR_THICKNESS; i++) {
      tft->drawRoundRect(px - i, py - i, ICON_SIZE + 2 * i, ICON_SIZE + 2 * i,
                         SELECTOR_RADIUS, BACKGROUND_COLOR);
    }

    // Draw new selector
    int row = selectedIndex / ITEMS_PER_ROW;
    int col = selectedIndex % ITEMS_PER_ROW;
    int x = LEFT_MARGIN + col * (ICON_SIZE + H_SPACING);
    int y = TOP_MARGIN + row * (ICON_SIZE + MARGIN_Y);

    for (int i = 0; i < SELECTOR_THICKNESS; i++) {
      tft->drawRoundRect(x - i, y - i, ICON_SIZE + 2 * i, ICON_SIZE + 2 * i,
                         SELECTOR_RADIUS, TFT_WHITE);
    }

    previousIndex = selectedIndex;
    lastInput = millis();
  }
}

// ###################### Launch Games by Name ######################
void GameMenu::launchGameByName(const char *name) {
  ::launchGameByName(name); // Calls the centralized version
}
