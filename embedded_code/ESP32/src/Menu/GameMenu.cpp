// GameMenu.cpp

#include "GameMenu.hpp"
#include <JPEGDecoder.h>
#include <SD.h>

#include "Bluetooth/BluetoothManager.hpp"
#include "Core/AppState.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "Menu/GameSetup.hpp"
#include "SettingsMenu/Settings/Settings.hpp"

using namespace MenuLayout; // Use layout constants
static bool inSettingsMode = false;

// ###################### Initialize Buttons ######################
GameMenu::GameMenu(TFT_eSPI *tft) : tft(tft), selectedIndex(0), drawer(*tft) {
  // initSettings(tft);
  // initBrightness(tft);

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
  static bool inSettingsMode = false;
  static int lastIndexBeforeSettings = 0;

  if (millis() - lastInput < 200)
    return;

  bool moved = false;

  // ========== Handle Gear Mode ==========
  if (inSettingsMode) {
    if (down.isPressed()) {
      // Clear selector from gear
      int gx = tft->width() - 32 - 15;
      int gy = 10; // slightly higher for visual alignment

      for (int i = 0; i < SELECTOR_THICKNESS; i++) {
        tft->drawRoundRect(gx - i, gy - i, 32 + 2 * i, 32 + 2 * i,
                           SELECTOR_RADIUS, BACKGROUND_COLOR);
      }

      // Exit gear mode
      selectedIndex = lastIndexBeforeSettings;
      inSettingsMode = false;
      moved = true;
    }

    if (A.wasJustPressed()) {
      playPressSound();
      runSettings();
      // Redraw menu after returning
      drawPage();
      selectedIndex = lastIndexBeforeSettings;
      inSettingsMode = false;
      moved = true;
      lastInput = millis();
      return;
    }
  }

  // ========== Handle Icon Navigation ==========
  else {
    if (up.isPressed()) {
      if (selectedIndex < ITEMS_PER_ROW) {
        // Going into gear
        lastIndexBeforeSettings = selectedIndex;
        inSettingsMode = true;
        moved = true;

        // Clear selector from icon
        int prevRow = selectedIndex / ITEMS_PER_ROW;
        int prevCol = selectedIndex % ITEMS_PER_ROW;
        int px = LEFT_MARGIN + prevCol * (ICON_SIZE + H_SPACING);
        int py = TOP_MARGIN + prevRow * (ICON_SIZE + MARGIN_Y);

        for (int i = 0; i < SELECTOR_THICKNESS; i++) {
          tft->drawRoundRect(px - i, py - i, ICON_SIZE + 2 * i,
                             ICON_SIZE + 2 * i, SELECTOR_RADIUS,
                             BACKGROUND_COLOR);
        }
      } else {
        selectedIndex -= ITEMS_PER_ROW;
        moved = true;
      }
    } else if (down.isPressed()) {
      if (selectedIndex + ITEMS_PER_ROW < ITEM_COUNT) {
        selectedIndex += ITEMS_PER_ROW;
        moved = true;
      }
    } else if (left.isPressed()) {
      if (selectedIndex == 0) {
        selectedIndex = ITEM_COUNT - 1;
      } else {
        selectedIndex--;
      }
      moved = true;
    } else if (right.isPressed()) {
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

    if (A.wasJustPressed()) {
      playPressSound();
      launchGameByName(gameBoxes[selectedIndex].name);
      drawPage();
      lastInput = millis();
      return;
    }
  }

  // ========== Draw Selector ==========
  if (moved) {
    if (inSettingsMode) {
      // Draw selector around gear
      int gx = tft->width() - 32 - 15;
      int gy = 10; // higher for alignment

      for (int i = 0; i < SELECTOR_THICKNESS; i++) {
        tft->drawRoundRect(gx - i, gy - i, 32 + 2 * i, 32 + 2 * i,
                           SELECTOR_RADIUS, TFT_WHITE);
      }
    } else {
      // Clear previous selector
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
    }

    lastInput = millis();
    playSelectBeep();
  }
}

// ###################### Launch Games by Name ######################
void GameMenu::launchGameByName(const char *name) {

  // Stop scanning before launching game
  BluetoothManager::stopScan();

  // // Update state if you use it for gameplay tracking
  // currentMenuState = STATE_GAME_RUNNING;

  ::launchGameByName(name); // Calls the centralized version (launch the game)
}