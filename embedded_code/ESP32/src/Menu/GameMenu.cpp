// GameMenu.cpp

#include "GameMenu.hpp"

bool shouldExitToMenu = false;

using namespace MenuLayout; // Use layout constants
static bool inSettingsMode = false;

// ###################### Initialize Buttons ######################
GameMenu::GameMenu(TFT_eSPI *tft) : tft(tft), selectedIndex(0), drawer(*tft) {

  // ###################### Lists Games ######################
  // ---------- Page 1 ----------
  gameBoxes[0] = {"Snake", 0, 0};
  gameBoxes[1] = {"Pong", 0, 1};
  gameBoxes[2] = {"Tic Tac Toe", 0, 2};
  gameBoxes[3] = {"Simon", 0, 3};
  gameBoxes[4] = {"Connect 4", 1, 0};
  gameBoxes[5] = {"Breakout", 1, 1};
  gameBoxes[6] = {"Memory", 1, 2};
  gameBoxes[7] = {"Tetris", 1, 3};

  // ---------- Page 2 ----------
  gameBoxes[8] = {"Chess", 0, 0}; // Top row
  gameBoxes[9] = {"Checkers", 0, 1};
  gameBoxes[10] = {"UNO", 0, 2};

  // ====== Show a notification if any new badge was unlocked ======
  if (hasPendingNotification) {
    notification.show(pendingNotificationMessage, pendingNotificationDuration);
    hasPendingNotification = false; // reset
  }
}

// ###################### Draw Screen ######################
void GameMenu::draw() { drawPage(); }

// ###################### Draw Menu Interface ######################
void GameMenu::drawPage() {
  tft->fillScreen(BACKGROUND_COLOR);

  // Select the appropriate background image for the current page
  const char *bgPath = (currentPage == 0) ? "/menu/assets/Background.jpg"
                                          : "/menu/assets/Background1.jpg";

  File jpegFile = SD.open(bgPath);
  if (!jpegFile) {
    Serial.print("Menu background not found: ");
    Serial.println(bgPath);
    return;
  }

  tft->setSwapBytes(true);

  if (JpegDec.decodeSdFile(jpegFile)) {
    while (JpegDec.read()) {
      int x = JpegDec.MCUx * JpegDec.MCUWidth;
      int y = JpegDec.MCUy * JpegDec.MCUHeight;
      tft->pushImage(x, y, JpegDec.MCUWidth, JpegDec.MCUHeight, JpegDec.pImage);
    }
  }
  jpegFile.close();

  // ================= Draw Game Names for This Page ================= //
  const int offset = (currentPage == 0) ? 0 : page1Count;
  const int count = (currentPage == 0) ? page1Count : page2Count;

  tft->setTextColor(TFT_WHITE, BACKGROUND_COLOR);
  tft->setTextSize(1);
  tft->setTextDatum(MC_DATUM);

  for (int i = 0; i < count; i++) {
    int row = i / ITEMS_PER_ROW;
    int col = i % ITEMS_PER_ROW;
    int x = LEFT_MARGIN + col * (ICON_SIZE + H_SPACING);
    int y = TOP_MARGIN + row * (ICON_SIZE + MARGIN_Y);
    int textX = x + ICON_SIZE / 2;
    int textY = y + ICON_SIZE + 10;

    tft->drawString(gameBoxes[offset + i].name, textX, textY);
  }

  // ================= Draw Selector ================= //
  int row = selectedIndex / ITEMS_PER_ROW;
  int col = selectedIndex % ITEMS_PER_ROW;
  int x = LEFT_MARGIN + col * (ICON_SIZE + H_SPACING);
  int y = TOP_MARGIN + row * (ICON_SIZE + MARGIN_Y);

  for (int i = 0; i < SELECTOR_THICKNESS; i++) {
    tft->drawRoundRect(x - i, y - i, ICON_SIZE + 2 * i, ICON_SIZE + 2 * i,
                       SELECTOR_RADIUS, TFT_WHITE);
  }

  processPendingNotification();
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
      previousIndex = selectedIndex;

      drawing.clearCache();
      drawing.clearSprite();
      drawing.deleteSprite();
      selectedIndex = lastIndexBeforeSettings;
      inSettingsMode = false;
      moved = true;
      lastInput = millis();
      return;
    }
  }

  // ========== Handle Icon Navigation ==========
  else {

    // ================== Handle UP =================== //
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
    }

    // ================== Handle DOWN =================== //
    else if (down.isPressed()) {
      int pageItemCount =
          (currentPage == 0) ? MenuLayout::page1Count : MenuLayout::page2Count;
      if (selectedIndex + MenuLayout::ITEMS_PER_ROW < pageItemCount) {
        selectedIndex += MenuLayout::ITEMS_PER_ROW;
        moved = true;
      }
    }

    // ================== Handle LEFT =================== //
    else if (left.isPressed()) {
      if (currentPage == 0 && selectedIndex == 0) {
        int prevRow = selectedIndex / ITEMS_PER_ROW;
        int prevCol = selectedIndex % ITEMS_PER_ROW;
        int px = LEFT_MARGIN + prevCol * (ICON_SIZE + H_SPACING);
        int py = TOP_MARGIN + prevRow * (ICON_SIZE + MARGIN_Y);
        for (int i = 0; i < SELECTOR_THICKNESS; i++) {
          tft->drawRoundRect(px - i, py - i, ICON_SIZE + 2 * i,
                             ICON_SIZE + 2 * i, SELECTOR_RADIUS,
                             BACKGROUND_COLOR);
        }

        currentPage = 1;
        selectedIndex = 2;
        drawPage();
        previousIndex = selectedIndex;
        return;
      } else if (currentPage == 0) {
        selectedIndex--;
      } else if (currentPage == 1 && selectedIndex == 0) {
        // From "Chess" → back to "Tetris"
        int prevRow = selectedIndex / ITEMS_PER_ROW;
        int prevCol = selectedIndex % ITEMS_PER_ROW;
        int px = LEFT_MARGIN + prevCol * (ICON_SIZE + H_SPACING);
        int py = TOP_MARGIN + prevRow * (ICON_SIZE + MARGIN_Y);

        for (int i = 0; i < SELECTOR_THICKNESS; i++) {
          tft->drawRoundRect(px - i, py - i, ICON_SIZE + 2 * i,
                             ICON_SIZE + 2 * i, SELECTOR_RADIUS,
                             BACKGROUND_COLOR);
        }

        currentPage = 0;
        selectedIndex = 7; // Tetris
        drawPage();
        previousIndex = selectedIndex;
        return;
      } else if (currentPage == 1 && selectedIndex > 0) {
        selectedIndex--;
      }

      moved = true;
    }

    // ================== Handle RIGHT =================== //
    else if (right.isPressed()) {
      if (currentPage == 0 && selectedIndex == page1Count - 1) {
        // Clear old selector before changing
        int prevRow = selectedIndex / ITEMS_PER_ROW;
        int prevCol = selectedIndex % ITEMS_PER_ROW;
        int px = LEFT_MARGIN + prevCol * (ICON_SIZE + H_SPACING);
        int py = TOP_MARGIN + prevRow * (ICON_SIZE + MARGIN_Y);
        for (int i = 0; i < SELECTOR_THICKNESS; i++) {
          tft->drawRoundRect(px - i, py - i, ICON_SIZE + 2 * i,
                             ICON_SIZE + 2 * i, SELECTOR_RADIUS,
                             BACKGROUND_COLOR);
        }

        // Then switch page and draw
        currentPage = 1;
        selectedIndex = 0;
        drawPage();
        previousIndex = selectedIndex;
        return;
      } else if (currentPage == 0 && selectedIndex < page1Count - 1) {
        selectedIndex++;
      } else if (currentPage == 1 && selectedIndex == page2Count - 1) {
        // From "UNO" → back to "Snake"
        currentPage = 0;
        selectedIndex = 0;
        drawPage(); // Redraw new background
        previousIndex = selectedIndex;
        return;
      } else if (currentPage == 1) {
        selectedIndex++;
      }
      moved = true;
    }

    // ================== Handle A =================== //
    if (A.wasJustPressed()) {
      playPressSound();
      int actualIndex =
          (currentPage == 0) ? selectedIndex : page1Count + selectedIndex;
      launchGameByName(gameBoxes[actualIndex].name);
      drawPage();
      previousIndex = selectedIndex;
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

  ::launchGameByName(name); // Calls the centralized version (launch the game)
}