// /src/HostGame/HostGame.cpp

#include "HostGame.hpp"
#include "BackButton/BackButton.hpp"

extern TFT_eSPI tft;
static String lastCode = "";
static String lastStatus = "";

// void HostGame::init(TFT_eSPI &display) { screen = display; }

void HostGame::showCode(const std::string &code, int selection, std::vector<String> &playerNames, bool host) {

  // lastCode = code;
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);

  // === Title ===
  tft.setTextSize(3);
  tft.setTextColor(TFT_CYAN, TFT_BLACK);
  tft.drawString("Hosting Game", tft.width() / 2, 25);

  // === Game Code Label ===
  tft.setTextSize(2);
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Game Code:", tft.width() / 2, 60);

  // === Game Code ===
  tft.setTextSize(5);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString(code.c_str(), tft.width() / 2, 120);

  // === Players Header ===
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE, TFT_BLACK);
  tft.drawString("Players Joined:", tft.width() / 2, 180);

  // === Player Names in 2 Rows, 4 Columns ===
  tft.setTextSize(2);
  int nameBoxWidth = tft.width() / 4;
  int rowY[2] = {210, 235}; // Y positions for row 1 and 2

  for (size_t i = 0; i < playerNames.size() && i < 8; ++i) {
    int col = i % 4;
    int row = i / 4;
    int x = nameBoxWidth * col + nameBoxWidth / 2;
    int y = rowY[row];
    tft.drawString(playerNames[i], x, y);
  }
  
  back(selection, TFT_BLACK,"< Back");

  // Only displayed to host
  if(host){
    // === Start Prompt ===
    const int btnWidth = 220;
    const int btnHeight = 30;
    const int btnX = (tft.width() - btnWidth) / 2;
    const int btnY = tft.height() - 45;
    const int btnRadius = 8;
  
    // Draw the prompt text always
    tft.setTextSize(2);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.drawString("Press to start", tft.width() / 2, btnY + btnHeight / 2);
  
    if (selection == 0) {
      // Erase any previously drawn rectangle only (not the text)
      tft.drawRoundRect(btnX, btnY, btnWidth, btnHeight, btnRadius, TFT_BLACK);
    } else {
      // Draw the rounded rectangle around the text
      tft.drawRoundRect(btnX, btnY, btnWidth, btnHeight, btnRadius, TFT_WHITE);
    }
  }


  delay(250);
  // updateAllButtons();
}

void HostGame::showStatus(const String &msg) {
  // if (!tft)
  //   return;

  lastStatus = msg;

  tft.setTextColor(TFT_YELLOW, TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  tft.drawString(msg, tft.width() / 2, 220); // Below the code
}

void HostGame::loopUntilConnected() {
  while (true) {
    updateAllButtons();

    if (checkStartButtonAndExit(tft)) {
      break; // return to game menu
    }

    // Add small delay to avoid burning CPU
    delay(50);
  }
}