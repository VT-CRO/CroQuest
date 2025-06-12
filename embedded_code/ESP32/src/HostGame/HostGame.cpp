// /src/HostGame/HostGame.cpp

#include "HostGame.hpp"

static TFT_eSPI *screen = nullptr;
static String lastCode = "";
static String lastStatus = "";

void HostGame::init(TFT_eSPI &display) { screen = &display; }

void HostGame::showCode(const String &code) {
  if (!screen)
    return;

  lastCode = code;

  screen->fillScreen(TFT_BLACK);
  screen->setTextColor(TFT_WHITE, TFT_BLACK);
  screen->setTextDatum(MC_DATUM);

  screen->setTextSize(3);
  screen->drawString("Hosting Game", screen->width() / 2, 60);

  screen->setTextSize(2);
  screen->drawString("Your Code:", screen->width() / 2, 120);

  screen->setTextSize(5);
  screen->drawString(code, screen->width() / 2, 170);

  screen->setTextSize(2);
  screen->drawString("Waiting for players...", screen->width() / 2, 240);

  delay(100);
  updateAllButtons();

  // === Timeout logic === //
  const unsigned long timeoutDuration = 3000; // 3seconds
  unsigned long startTime = millis();

  while (!getExitFlag()) {
    updateAllButtons();
    checkStartButtonAndExit(*screen);

    // Check for timeout
    if (millis() - startTime > timeoutDuration) {
      screen->setTextSize(1);
      screen->drawString("Press Start to return to menu", screen->width() / 2,
                         screen->height() - 20);
      shouldExitToMenu = true;
      break;
    }

    delay(50);
  }
}

void HostGame::showStatus(const String &msg) {
  if (!screen)
    return;

  lastStatus = msg;

  screen->setTextColor(TFT_YELLOW, TFT_BLACK);
  screen->setTextDatum(MC_DATUM);
  screen->setTextSize(2);
  screen->drawString(msg, screen->width() / 2, 220); // Below the code
}

void HostGame::loopUntilConnected() {
  while (true) {
    updateAllButtons();

    if (checkStartButtonAndExit(*screen)) {
      break; // return to game menu
    }

    // Add small delay to avoid burning CPU
    delay(50);
  }
}