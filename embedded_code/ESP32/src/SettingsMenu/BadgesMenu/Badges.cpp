// src/SettingsMenu/BadgesMenu/Badges.cpp

#include "SettingsMenu/BadgesMenu/Badges.hpp"

void runBadgesMenu() {

  const int badgeCount = 10;
  const int badgeSize = 64;
  const int spacing = 16;
  const int rowCount = 2;
  const int colCount = 5;
  const int descBoxHeight = 40;
  const int footerHeight = 10;
  const int bottomPadding = 20;

  const int gridHeight =
      rowCount * badgeSize + (rowCount - 1) * spacing + bottomPadding;
  const int startY =
      (tft.height() - gridHeight - descBoxHeight - footerHeight) / 2;
  const int startX =
      (tft.width() - (colCount * badgeSize + (colCount - 1) * spacing)) / 2;

  const char *badgePaths[badgeCount] = {
      "/badges/level1.jpg", "/badges/level2.jpg", "/badges/level3.jpg",
      "/badges/level4.jpg", "/badges/level5.jpg", "/badges/level6.jpg",
      "/badges/level7.jpg", "/badges/level8.jpg", "/badges/level9.jpg",
      "/badges/level10.jpg"};

  // TODO: Add the actual badge images to the SD card in the /badges directory
  const char *badgeDescriptions[badgeCount] = {
      "Change your name.",
      "Eat 150 apples in Snake",
      "Perfect Pong 5 times.",
      "Perfect Tic Tac Toe 5 times.",
      "Reach Level 25 in Simon",
      "Win 10 matches of Connect 4.",
      "Perfect Breakout 5 times",
      "Win Matching in less than 150 seconds.",
      "Reach 1000 points in Tetris.",
      "Get all 9 badges!"};

  int selectedIndex = 0;

  // Description Box variables
  int descX = 20;
  int descY = tft.height() - descBoxHeight - 20;
  int descW = tft.width() - 40;
  int descH = descBoxHeight;

  auto drawBadges = [&]() {
    tft.fillScreen(SETTINGS_BG_COLOR);
    tft.setTextDatum(TC_DATUM);
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.drawString("Your Badges", tft.width() / 2, 20);

    for (int i = 0; i < badgeCount; i++) {
      int row = i / colCount;
      int col = i % colCount;
      int x = startX + col * (badgeSize + spacing);
      int y = startY + row * (badgeSize + spacing);

      drawSdJpeg(badgePaths[i], x, y);

      // Selector border
      if (i == selectedIndex) {
        for (int j = 0; j < 3; j++) {
          tft.drawRoundRect(x - j, y - j, badgeSize + 2 * j, badgeSize + 2 * j,
                            4, TFT_WHITE);
        }

        // ==== Description Box ====

        // Draw white border around the description box
        tft.drawRoundRect(descX - 1, descY - 1, descW + 2, descH + 2, 6,
                          TFT_WHITE);

        // Fill balck background inside
        tft.fillRect(descX, descY, descW, descH, TFT_BLACK);
        tft.setTextDatum(CC_DATUM);
        tft.setTextColor(TFT_WHITE);
        tft.setTextSize(1);
        tft.drawString(badgeDescriptions[i], tft.width() / 2,
                       tft.height() - descBoxHeight);
      }
    }

    tft.setTextDatum(BC_DATUM);
    tft.setTextSize(1);
    tft.drawString("[A] to go back", tft.width() / 2, tft.height() - 5);
  };

  drawBadges();

  while (true) {
    if (left.wasJustPressed()) {
      selectedIndex = (selectedIndex - 1 + badgeCount) % badgeCount;
      drawBadges();
      delay(150);
    } else if (right.wasJustPressed()) {
      selectedIndex = (selectedIndex + 1) % badgeCount;
      drawBadges();
      delay(150);
    } else if (A.wasJustPressed()) {
      break;
    }

    delay(10);
  }
}
