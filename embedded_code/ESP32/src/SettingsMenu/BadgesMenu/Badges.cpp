// src/SettingsMenu/BadgesMenu/Badges.cpp

#include "SettingsMenu/BadgesMenu/Badges.hpp"

// ####################################################################################################
//  Global Definitions
// ####################################################################################################

const int badgeSize = 90;
const int spacing = 25;
const int rowCount = 2;
const int colCount = 4;
const int rowSpacing = 22;
const int descBoxHeight = 25;
const int footerHeight = 10;

const int topPadding = 10;
const int bottomPadding = 20;
const int leftPadding = 30;
const int rightPadding = 10;

const int page1Count = 8;
const int page2Count = 1;
int currentPage = 0;

bool badgeProgress[badgeCount];
bool isUnlocked[badgeCount] = {false};
bool allBadgesEarned = false;

static int startX = 0;
static int startY = 0;
static int gridHeight = 0;

const char *badgePaths[badgeCount] = {
    "/badges/assets/0.jpg", "/badges/assets/1.jpg", "/badges/assets/2.jpg",
    "/badges/assets/3.jpg", "/badges/assets/4.jpg", "/badges/assets/5.jpg",
    "/badges/assets/6.jpg", "/badges/assets/7.jpg", "/badges/assets/8.jpg"};
// "/badges/assets/9.jpg"};

const char *badgeDescriptions[badgeCount] = {

    "Eat 150 apples in Snake", "Perfect Pong", "Perfect Tic Tac Toe 3 times",
    "Reach Level 25 in Simon", "Win 3 matches in a row in Connect 4",
    "Perfect Breakout 3 times", "Win Matching in less than 150 seconds",
    "Reach 5000 points in Tetris",
    // "Captured the Queen",
    "Get all 8 badges!"};

// ####################################################################################################
//  Setup
// ####################################################################################################

// ========== Run Badges <enu ========== //
void runBadgesMenu() {
  loadBadgeProgress();

  gridHeight = rowCount * badgeSize + (rowCount - 1) * spacing + bottomPadding;
  startY = (tft.height() - gridHeight - descBoxHeight - footerHeight) / 2 +
           topPadding;
  startX = ((tft.width() - leftPadding - rightPadding) -
            (colCount * badgeSize + (colCount - 1) * spacing)) /
               2 +
           leftPadding - 30;

  int selectedIndex = 0;
  int xOffset = 15;
  int yOffset = 20;

  const int descX = 20;
  const int descY = tft.height() - descBoxHeight - 17;
  const int descW = tft.width() - 40;
  const int descH = descBoxHeight;

  const int extraWidth = 10;
  const int extraHeight = 10;

  drawBadges(selectedIndex, xOffset, yOffset, extraWidth, extraHeight, descX,
             descY, descW, descH);

  while (true) {
    int previousIndex = selectedIndex;
    int pageCount = (currentPage == 0) ? page1Count : page2Count;

    // ========== LEFT ========== //
    if (left.wasJustPressed()) {
      if (selectedIndex % colCount > 0) {
        selectedIndex--;
      } else if (currentPage == 0 && selectedIndex == 0) {
        currentPage = 1;
        selectedIndex = 8; // Final badge
        drawBadges(selectedIndex, xOffset, yOffset, extraWidth, extraHeight,
                   descX, descY, descW, descH);
        drawSelectorAndDescription(selectedIndex, -1, xOffset, yOffset,
                                   extraWidth, extraHeight, descX, descY, descW,
                                   descH);
      } else if (currentPage == 1) {
        currentPage = 0;
        selectedIndex = 7;
        drawBadges(selectedIndex, xOffset, yOffset, extraWidth, extraHeight,
                   descX, descY, descW, descH);
        drawSelectorAndDescription(selectedIndex, -1, xOffset, yOffset,
                                   extraWidth, extraHeight, descX, descY, descW,
                                   descH);
      }
      playSelectBeep();
    }

    // ========== RIGHT ========== //
    else if (right.wasJustPressed()) {
      int pageEnd = (currentPage == 0) ? page1Count : badgeCount;
      if ((selectedIndex + 1) % colCount != 0 && selectedIndex + 1 < pageEnd) {
        selectedIndex++;
      } else if (currentPage == 0) {
        currentPage = 1;
        selectedIndex = 8; // Final badge
        drawBadges(selectedIndex, xOffset, yOffset, extraWidth, extraHeight,
                   descX, descY, descW, descH);
        drawSelectorAndDescription(selectedIndex, -1, xOffset, yOffset,
                                   extraWidth, extraHeight, descX, descY, descW,
                                   descH);
      } else {
        currentPage = 0;
        selectedIndex = 0;
        drawBadges(selectedIndex, xOffset, yOffset, extraWidth, extraHeight,
                   descX, descY, descW, descH);
        drawSelectorAndDescription(selectedIndex, -1, xOffset, yOffset,
                                   extraWidth, extraHeight, descX, descY, descW,
                                   descH);
      }
      playSelectBeep();
    }

    // ========== UP ========== //
    else if (up.wasJustPressed()) {
      if (selectedIndex - colCount >= ((currentPage == 0) ? 0 : page1Count))
        selectedIndex -= colCount;
      playSelectBeep();
    }

    // ========== DOWN ========== //
    else if (down.wasJustPressed()) {
      if (selectedIndex + colCount <
          ((currentPage == 0) ? page1Count : badgeCount))
        selectedIndex += colCount;
      playSelectBeep();
    }

    // ========== EXIT ========== //
    else if (B.wasJustPressed()) {
      break;
    }

    // ========== Redraw if changed ========== //
    if (selectedIndex != previousIndex) {
      drawSelectorAndDescription(selectedIndex, previousIndex, xOffset, yOffset,
                                 extraWidth, extraHeight, descX, descY, descW,
                                 descH);
      delay(150);
    }

    notification.update();
    delay(10);
  }
}

// ####################################################################################################
//  Logic
// ####################################################################################################

// ========== Load Badges ========== //
void loadBadgeProgress() {
  if (!SD.exists("/badges/save.dat")) {
    Serial.println("Badge save file not found. Initializing new progress.");
    for (int i = 0; i < badgeCount; i++) {
      badgeProgress[i] = false;
      isUnlocked[i] = false;
    }
    return;
  }

  File file = SD.open("/badges/save.dat", FILE_READ);
  if (!file) {
    Serial.println("Failed to open badge save file.");
    return;
  }

  for (int i = 0; i < badgeCount; i++) {
    int b = file.read();
    badgeProgress[i] = (b == 1);
    isUnlocked[i] = (b == 1);
  }

  file.close();
  Serial.println("Badge progress loaded.");

  // ========== Unlock final badge if all previous are earned ==========
  bool allUnlocked = true;
  for (int i = 0; i < 8; i++) { // Only check badges 0 through 7
    if (!badgeProgress[i]) {
      allUnlocked = false;
      break;
    }
  }

  if (allUnlocked && !badgeProgress[8]) { // Unlock final badge (index 8)
    badgeProgress[8] = true;
    isUnlocked[8] = true;

    hasPendingNotification = true;
    pendingNotificationMessage = "All Badges Unlocked!";
    pendingNotificationDuration = 3000;

    // Save updated progress to SD
    file = SD.open("/badges/save.dat", FILE_WRITE);
    if (file) {
      for (int i = 0; i < badgeCount; i++) {
        file.write(isUnlocked[i] ? 1 : 0);
      }
      file.close();
    }
  }

  // ========== Update global flag ==========
  allBadgesEarned = true;
  for (int i = 0; i < 8; i++) {
    if (!badgeProgress[i]) {
      allBadgesEarned = false;
      break;
    }
  }
}

// ========== Saves New Badge ========== //
void saveBadgeProgress() {
  if (!SD.exists("/badges")) {
    SD.mkdir("/badges"); // Ensure directory exists
  }

  File file = SD.open("/badges/save.dat", FILE_WRITE);
  if (!file) {
    Serial.println("❌ Failed to open badge save file for writing.");
    return;
  }

  for (int i = 0; i < badgeCount; i++) {
    file.write(isUnlocked[i] ? 1 : 0);
  }

  file.close();
  Serial.println("✅ Badge progress saved.");
}

void resetBadgeProgress() {
  File file = SD.open("/badges/save.dat", FILE_WRITE);
  if (!file) {
    Serial.println("❌ Failed to open badge save file to reset.");
    return;
  }

  for (int i = 0; i < badgeCount; i++) {
    file.write(0); // 0 = locked
    isUnlocked[i] = false;
    badgeProgress[i] = false;
  }

  file.close();
  Serial.println("✅ Badge progress reset.");
}

void checkFinalBadgeUnlock() {
  // Check if all core badges [0–7] are unlocked
  bool allUnlocked = true;
  for (int i = 0; i < 8; i++) {
    if (!badgeProgress[i]) {
      allUnlocked = false;
      break;
    }
  }

  // If final badge [8] is not yet unlocked
  if (allUnlocked && !badgeProgress[8]) {
    badgeProgress[8] = true;
    isUnlocked[8] = true;

    saveBadgeProgress(); // Persist new unlock state

    hasPendingNotification = true;
    pendingNotificationMessage = "All Badges Unlocked!";
    pendingNotificationDuration = 3000;
  }
}

// ####################################################################################################
//  Drawing
// ####################################################################################################

// ========== Draw the Badges ========== //
void drawBadges(int selectedIndex, int xOffset, int yOffset, int extraWidth,
                int extraHeight, int descX, int descY, int descW, int descH) {

  // Golden Selector
  uint16_t selectorColor = allBadgesEarned ? 0xFFD700 : TFT_WHITE;

  tft.fillScreen(SETTINGS_BG_COLOR);

  // Clear badge area
  tft.fillRect(0, 0, tft.width(), tft.height() - descBoxHeight - footerHeight,
               SETTINGS_BG_COLOR);

  tft.setTextDatum(TC_DATUM);
  tft.setTextColor(selectorColor);
  tft.setTextSize(2);
  tft.drawString("BADGES", tft.width() / 2, 20);

  int start = (currentPage == 0) ? 0 : page1Count;
  int end = (currentPage == 0) ? page1Count : badgeCount;
  int localIndex = 0;

  for (int i = start; i < end; i++) {
    int row = localIndex / colCount;
    int col = localIndex % colCount;
    int x = startX + col * (badgeSize + spacing);
    int y = startY + row * (badgeSize + rowSpacing);

    const char *path =
        isUnlocked[i] ? badgePaths[i] : "/badges/assets/empty.jpg";
    drawSdJpeg(path, x, y);

    localIndex++;
  }

  if (selectedIndex >= start && selectedIndex < end) {
    int localIndex = selectedIndex - start;
    int selRow = localIndex / colCount;
    int selCol = localIndex % colCount;
    int selX = startX + selCol * (badgeSize + spacing);
    int selY = startY + selRow * (badgeSize + rowSpacing);

    int selectorBottom = selY + badgeSize + extraHeight;
    int descTop = descY - 2;

    if (selectorBottom < descTop) {
      for (int j = 0; j < 3; j++) {
        tft.drawRoundRect(selX - j + xOffset - extraWidth / 2,
                          selY - j + yOffset - extraHeight / 2,
                          badgeSize + 2 * j + extraWidth,
                          badgeSize + 2 * j + extraHeight, 6, selectorColor);
      }
    }
  }

  tft.drawRoundRect(descX - 1, descY - 1, descW + 2, descH + 2, 6,
                    selectorColor);
  tft.fillRect(descX, descY, descW, descH, TFT_BLACK);
  tft.setTextDatum(CC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.drawString(badgeDescriptions[selectedIndex], tft.width() / 2,
                 tft.height() - descBoxHeight);

  tft.setTextDatum(BC_DATUM);
  tft.setTextSize(1);
  tft.drawString("[B] to go back", tft.width() / 2, tft.height() - 5);
}

// ========== Draw Selector ========== //
void drawSelectorAndDescription(int index, int prevIndex, int xOffset,
                                int yOffset, int extraWidth, int extraHeight,
                                int descX, int descY, int descW, int descH) {

  // Golden Selector
  uint16_t selectorColor = allBadgesEarned ? 0xFFD700 : TFT_WHITE;

  int descTop = descY - 2;

  // ===== Clear previous selector =====
  if (prevIndex >= 0) {
    int localPrev = prevIndex - ((currentPage == 0) ? 0 : page1Count);
    int pr = localPrev / colCount;
    int pc = localPrev % colCount;

    int px = startX + pc * (badgeSize + spacing);
    int py = startY + pr * (badgeSize + rowSpacing);

    int selectorBottom = py + badgeSize + extraHeight;

    if (selectorBottom < descTop) {
      tft.drawRoundRect(px + xOffset - extraWidth / 2,
                        py + yOffset - extraHeight / 2, badgeSize + extraWidth,
                        badgeSize + extraHeight, 6, SETTINGS_BG_COLOR);
    }

    if ((currentPage == 0 && prevIndex < page1Count) ||
        (currentPage == 1 && prevIndex >= page1Count)) {
      const char *path = isUnlocked[prevIndex] ? badgePaths[prevIndex]
                                               : "/badges/assets/empty.jpg";
      drawSdJpeg(path, px, py);
    }
  }

  // ===== Draw new selector =====
  int localIndex = index - ((currentPage == 0) ? 0 : page1Count);
  int row = localIndex / colCount;
  int col = localIndex % colCount;

  int x = startX + col * (badgeSize + spacing);
  int y = startY + row * (badgeSize + rowSpacing);

  for (int j = 0; j < 3; j++) {
    int borderY = y - j + yOffset - extraHeight / 2;
    int borderH = badgeSize + 2 * j + extraHeight;

    if (borderY + borderH < descTop) {
      tft.drawRoundRect(x - j + xOffset - extraWidth / 2, borderY,
                        badgeSize + 2 * j + extraWidth, borderH, 6,
                        selectorColor);
    }
  }

  // ===== Update description box =====
  tft.fillRect(descX, descY, descW, descH, TFT_BLACK);
  tft.setTextDatum(CC_DATUM);
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(1);
  tft.drawString(badgeDescriptions[index], tft.width() / 2, descY + descH / 2);
}
