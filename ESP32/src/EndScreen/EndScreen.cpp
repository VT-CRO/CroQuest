// src/EndScreen/EndScreen.cpp

#include "EndScreen.hpp"

inline EndScreen::Selection operator++(EndScreen::Selection &s, int) {
  typedef typename std::underlying_type<EndScreen::Selection>::type T;
  EndScreen::Selection old = s;
  if (s < EndScreen::GAME_MENU) {
    s = static_cast<EndScreen::Selection>(static_cast<T>(s) + 1);
  }
  return old;
}

inline EndScreen::Selection operator--(EndScreen::Selection &s, int) {
  typedef typename std::underlying_type<EndScreen::Selection>::type T;
  EndScreen::Selection old = s;
  if (s > EndScreen::BACK_BUTTON) {
    s = static_cast<EndScreen::Selection>(static_cast<T>(s) - 1);
  }
  return old;
}

// ========== End Screen ========== //
void EndScreen::gameOverScreen() {
  // Clear screen
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(4);

  // Determine highest score and how many players have it
  int highScore = INT_MIN;
  int highScoreCount = 0;
  for (int s : playerScores) {
    if (s > highScore) {
      highScore = s;
      highScoreCount = 1;
    } else if (s == highScore) {
      highScoreCount++;
    }
  }

  // Determine game result
  String resultText;
  uint16_t resultColor;

  if (playerNames.size() > 1) {
    if (score == highScore) {
      if (highScoreCount == 1) {
        resultText = "YOU WON!";
        resultColor = TFT_GREEN;
      } else {
        resultText = "YOU TIED";
        resultColor = TFT_ORANGE;
      }
    } else {
      resultText = "YOU LOST";
      resultColor = TFT_RED;
    }
  } else {
    resultText = "GAME OVER";
    resultColor = TFT_RED;
  }

  // --- Display result banner ---
  int centerX = tft.width() / 2;

  tft.setTextColor(resultColor);
  tft.setTextSize(6);
  tft.drawString(resultText, centerX, 70);

  // Draw decorative line under banner
  tft.drawLine(40, 110, tft.width() - 40, 110, resultColor);

  // --- Display player name and score ---
  tft.setTextSize(2);
  int nameY = tft.height() / 2;
  int scoreY = nameY + 35;

  String nameStr = String(playerName);
  tft.setTextColor((multiplayer && score == highScore && highScoreCount == 1)
                       ? TFT_GREEN
                       : TFT_YELLOW);
  tft.drawString(nameStr, centerX, nameY);

  // Underline name
  int charWidth = 6 * 2; // textSize(2)
  int textWidth = nameStr.length() * charWidth;
  int underlineY = nameY + 14;
  tft.drawLine(
      centerX - textWidth / 2, underlineY, centerX + textWidth / 2, underlineY,
      (multiplayer && score == highScore && highScoreCount == 1) ? TFT_GREEN
                                                                 : TFT_YELLOW);

  if (score >= 0) {
    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Score: " + String(score), centerX, scoreY);
  }

  // Footer options or selection
  drawingSelections(TFT_BLACK);
}

// ========== Score Board ========== //
void EndScreen::scoreBoardScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);

  // Draw title
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.drawString("SCOREBOARD", tft.width() / 2, 30);
  tft.setTextSize(2);

  tft.setTextDatum(TL_DATUM); // Top-left corner datum for clean layout

  // Table headers
  int startX = 20;
  int nameX = startX;
  int scoreX = tft.width() - 80;
  int startY = 50;
  int rowHeight = 28;

  tft.setTextColor(TFT_LIGHTGREY);
  tft.drawString("PLAYER", nameX, startY);
  tft.drawString("SCORE", scoreX, startY);

  // Line under headers
  tft.drawLine(startX, startY + 22, tft.width() - 20, startY + 22,
               TFT_LIGHTGREY);

  // Determine high score
  int highScore = 0;
  for (int score : playerScores) {
    if (score > highScore)
      highScore = score;
  }

  // Print each row
  for (int i = 0; i < playerNames.size(); ++i) {
    int rowY = startY + 30 + i * rowHeight;

    // Sets text color
    tft.setTextColor(playerScores[i] == highScore ? TFT_GREEN : TFT_WHITE);

    String displayName = playerNames[i];

    // If it's the local user, format name and add (you)
    if (playerNames[i] == settings.name && playerScores[i] == score) {
      // Some of the games never have ties, so it's still useful to have the 
      // default "else" statement
      if(index != -1){
        // Makes sure the proper player is labeled (When there is a possibility for players
        // to tie, they may have the same name, thus name and score aren't enough)
        if(i == index){
          displayName.toLowerCase();
          displayName[0] = toupper(displayName[0]);
          displayName += "   (You)";
        }
      }else{
        displayName.toLowerCase();
        displayName[0] = toupper(displayName[0]);
        displayName += "   (You)";
      }
    } else {
      displayName.toLowerCase();
      displayName[0] = toupper(displayName[0]);
    }

    tft.drawString(displayName, nameX, rowY);
    tft.drawString(String(playerScores[i]), scoreX, rowY);
  }

  // Footer prompt
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("Press A for endscreen", tft.width() / 2, tft.height() - 30);
}

// ========== Handle User Input ========== //
bool EndScreen::handleUserInput() {
  if (multiplayer) {
    scoreBoardScreen();
  } else {
    gameOverScreen();
    currentState = ENDSCREEN;
  }

  unsigned long lastButtonPressTime = 0;
  unsigned long buttonDebounceDelay = 200;

  for (;;) {
    static int prevSelection = -1;

    if (millis() - lastButtonPressTime > buttonDebounceDelay) {
      if (checkStartButtonAndExit(tft)) {
        exit = true;
        return false;
      }

      switch (currentState) {
      case SCORE_BOARD:
        if (A.wasJustPressed()) {
          currentState = ENDSCREEN;
          gameOverScreen();
          lastButtonPressTime = millis();
        }
        break;

      case ENDSCREEN:
        // Selection logic
        if (A.wasJustPressed()) {
          if (currentSelection == RESTART_GAME) {
            currentState = SCORE_BOARD;
            currentSelection = RESTART_GAME;
            return true;
          } else if (currentSelection == BACK_BUTTON) {
            currentState = SCORE_BOARD;
            scoreBoardScreen();
          } else if (currentSelection == GAME_MENU) {
            currentState = SCORE_BOARD;
            currentSelection = RESTART_GAME;
            return false;
          }
          lastButtonPressTime = millis();
        } else if (up.isPressed()) {
          if (multiplayer) {
            if (currentSelection > BACK_BUTTON) {
              currentSelection--;
            }
          } else {
            if (currentSelection > RESTART_GAME) {
              currentSelection--;
            }
          }
          lastButtonPressTime = millis();
        } else if (down.isPressed()) {
          if (currentSelection < GAME_MENU) {
            currentSelection++;
          }
          lastButtonPressTime = millis();
        }

        // Only redraw when selection changes
        if (currentSelection != prevSelection) {
          drawingSelections(TFT_BLACK); // or pass a bgColor variable
          prevSelection = currentSelection;
        }

        break;
      }
    }
  }
}

// ========== Draw Seleciton Buttons ========== //
void EndScreen::drawingSelections(uint16_t bgcolor) {
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_WHITE);

  // --------- Y Positions --------- //
  int y_restart = tft.height() - 70;
  int y_menu = tft.height() - 30;

  // --------- Clear areas behind buttons --------- //
  tft.fillRect(0, y_restart - 20, tft.width(), 40, bgcolor);
  tft.fillRect(0, y_menu - 20, tft.width(), 40, bgcolor);

  // --------- "Press to Restart" --------- //
  int textSize_restart = (currentSelection == RESTART_GAME) ? 3 : 2;
  tft.setTextSize(textSize_restart);
  tft.drawString("Press to restart", tft.width() / 2, y_restart);

  // --------- "Press to Return to Menu" --------- //
  int textSize_menu = (currentSelection == GAME_MENU) ? 3 : 2;
  tft.setTextSize(textSize_menu);
  tft.drawString("Press to return to menu", tft.width() / 2, y_menu);

  // --------- Back Button for Scoreboard --------- //
  if (multiplayer) {
    back(currentSelection, bgcolor, "< Scores");
  }
}
