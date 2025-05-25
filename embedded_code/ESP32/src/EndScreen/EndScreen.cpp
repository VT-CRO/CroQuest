#include "EndScreen.hpp"
#include "BackButton/BackButton.hpp"

inline EndScreen::Selection operator++(EndScreen::Selection& s, int) {
    typedef typename std::underlying_type<EndScreen::Selection>::type T;
    EndScreen::Selection old = s;
    if (s < EndScreen::GAME_MENU) {
        s = static_cast<EndScreen::Selection>(static_cast<T>(s) + 1);
    }
    return old;
}

inline EndScreen::Selection operator--(EndScreen::Selection& s, int) {
    typedef typename std::underlying_type<EndScreen::Selection>::type T;
    EndScreen::Selection old = s;
    if (s > EndScreen::BACK_BUTTON) {
        s = static_cast<EndScreen::Selection>(static_cast<T>(s) - 1);
    }
    return old;
}


// ==================== ENDSCREEN ================== //

void EndScreen::gameOverScreen() {
    // Clear screen
    tft.fillScreen(TFT_BLACK);
    tft.setTextDatum(MC_DATUM);
    tft.setTextSize(4);

    // Determine highest score and how many players have it
    int highScore = 0;
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

    if (score == highScore) {
        if (highScoreCount == 1) {
            resultText = "YOU WON";
            resultColor = TFT_GREEN;
        } else {
            resultText = "YOU TIED";
            resultColor = TFT_ORANGE;
        }
    } else {
        resultText = "YOU LOST";
        resultColor = TFT_RED;
    }

    // Show result text
    tft.setTextColor(resultColor);
    tft.drawString(resultText, tft.width() / 2, 40);

    // Show player name and score
    tft.setTextSize(2);
    int centerX = tft.width() / 2;
    int nameY = tft.height() / 2 - 40;
    int scoreY = tft.height() / 2 - 10;

    String nameStr = String(playerName);
    tft.setTextColor(TFT_CYAN);
    tft.drawString(nameStr, centerX, nameY);

    // Underline name
    int charWidth = 6 * 2; // textSize(2)
    int textWidth = nameStr.length() * charWidth;
    int underlineY = nameY + 14;
    tft.drawLine(centerX - textWidth / 2, underlineY,
                 centerX + textWidth / 2, underlineY,
                 TFT_CYAN);

    tft.setTextColor(TFT_YELLOW);
    tft.drawString("Score: " + String(score), centerX, scoreY);

    drawingSelections(TFT_BLACK);
}


void EndScreen::scoreBoardScreen() {
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(MC_DATUM);
  tft.setTextSize(2);
  
  // Draw title
  tft.setTextColor(TFT_WHITE);
  tft.setTextSize(3);
  tft.drawString("SCOREBOARD", tft.width() / 2, 30);
  tft.setTextSize(2);

  tft.setTextDatum(TL_DATUM);  // Top-left corner datum for clean layout

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
  tft.drawLine(startX, startY + 22, tft.width() - 20, startY + 22, TFT_LIGHTGREY);

  // Determine high score
  int highScore = 0;
  for (int score : playerScores) {
    if (score > highScore) highScore = score;
  }

  // Print each row
  for (int i = 0; i < playerNames.size(); ++i) {
    int rowY = startY + 30 + i * rowHeight;

    // Sets text color
    tft.setTextColor(playerScores[i] == highScore ? TFT_GREEN : TFT_WHITE);

    tft.drawString(playerNames[i], nameX, rowY);
    tft.drawString(String(playerScores[i]), scoreX, rowY);
  }

  // Footer prompt
  tft.setTextDatum(MC_DATUM);
  tft.setTextColor(TFT_YELLOW);
  tft.drawString("Press A for endscreen", tft.width() / 2, tft.height() - 30);
}

// Handles user input
bool EndScreen::handleUserInput(){
    
    //Draws the scoreboard 
    if(multiplayer){
        scoreBoardScreen();
    }else{
        gameOverScreen();
        currentState = ENDSCREEN;
    }

    unsigned long lastButtonPressTime = 0;
    unsigned long buttonDebounceDelay = 200;
    for(;;){
        if (millis() - lastButtonPressTime > buttonDebounceDelay){
            switch(currentState){
                // SCOREBOARD DISPLAYED
                case SCORE_BOARD:
                    // GOES TO ENDSCREEN
                    if(A.wasJustPressed()){
                        currentState = ENDSCREEN;
                        gameOverScreen();
                        lastButtonPressTime = millis();
                    }
                    break;
                case ENDSCREEN:
                    // Selection logic
                    if(A.wasJustPressed()){
                        if(currentSelection == RESTART_GAME){
                            // GAME SHOULD RESTART
                            currentState = SCORE_BOARD;
                            currentSelection = RESTART_GAME;
                            return true;
                        }else if(currentSelection == BACK_BUTTON){
                            // SHOULD RETURN TO SCOREBOARD
                            currentState = SCORE_BOARD;
                            scoreBoardScreen();

                        }else if(currentSelection == GAME_MENU){
                            // GAME SHOULD RETURN TO GAME MENU
                            currentState = SCORE_BOARD;
                            currentSelection = RESTART_GAME;
                            return false;
                        }
                        lastButtonPressTime = millis();
                    }else if(up.isPressed()){
                        if(multiplayer){
                            if(currentSelection > BACK_BUTTON){
                                currentSelection--;
                            }
                        }else{
                            if(currentSelection > RESTART_GAME){
                                currentSelection--;
                            }
                        }
                        drawingSelections(TFT_BLACK);
                        lastButtonPressTime = millis();
                    }else if(down.isPressed()){
                        if(currentSelection < GAME_MENU){
                            currentSelection++;
                        }
                        drawingSelections(TFT_BLACK);
                        lastButtonPressTime = millis();
                    }
                    break;
            };
        }
    }
}

void EndScreen::drawingSelections(uint16_t bgcolor){
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    
    // PRESS TO RESTART
    tft.drawRect(tft.width() / 2 - tft.textWidth("Press to restart")/2 - 4, tft.height() - 60 - tft.fontHeight()/2 - 4, tft.textWidth("Press to restart") + 8,  
                    tft.fontHeight() + 8, currentSelection == RESTART_GAME ? TFT_WHITE : bgcolor);
    tft.drawString("Press to restart", tft.width() / 2, tft.height() - 60);
    
    // PRESS TO RETURN TO MENU
    tft.drawRect(tft.width() / 2 - tft.textWidth("Press to return to menu")/2 - 4, tft.height() - 30 - tft.fontHeight()/2 - 4, tft.textWidth("Press to return to menu") + 8,  
                    tft.fontHeight() + 8, currentSelection == GAME_MENU ? TFT_WHITE : bgcolor);
    tft.drawString("Press to return to menu", tft.width() / 2, tft.height() - 30);

    if(multiplayer){
        //Draws back button
        back(currentSelection, bgcolor, "< Scores");
    }
}