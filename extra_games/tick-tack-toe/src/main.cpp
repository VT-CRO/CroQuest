// // #include <TFT_eSPI.h>
// // #include "JpegDrawing.hpp"
// // #include "NumPad.hpp"
// #include "Buttons.hpp"

// // TFT_eSPI tft = TFT_eSPI();
// // JpegDrawing drawing(tft);
// // //Assets
// // const char* BOARD_PATH = "/tic_tac_toe_assets/board.jpg";
// // const char* X_PATH = "/tic_tac_toe_assets/x.jpg";
// // const char* O_PATH = "/tic_tac_toe_assets/o.jpg";
// // const char* DIS_O_PATH = "/tic_tac_toe_assets/disappearing_o.jpg";
// // const char* DIS_X_PATH = "/tic_tac_toe_assets/disappearing_x.jpg";

// // #define SCREEN_WIDTH 480
// // #define SCREEN_HEIGHT 320

// Button A(22, "A", DIGITAL);
// Button B(39, "B", DIGITAL);
// Button up(35, "Up", ANALOG_INPUT, 2000, 3600);
// Button right(35, "Right", ANALOG_INPUT, 3601, 4095);
// Button left(34, "Left", ANALOG_INPUT, 2000, 3600);
// Button down(34, "Down", ANALOG_INPUT, 3601, 4095);

// // #define SD_CS 5

// // void drawScoreboard();
// // void drawWinnerMessage();
// // void drawWinLine();
// // void checkWinner();
// // void clearCursor(int index);
// // void highlightCursor(int index);
// // void drawGrid();
// // void drawAllPlaying();
// // void drawEndScreen();
// // void drawHomeScreen();
// // void drawHomescreenSelect();

// // String board[9] = { "**", "**", "**", "**", "**", "**", "**", "**", "**" };
// // char currentPlayer = 'X';
// // char winner = 'N';  // 'X', 'O', 'D' (draw), or 'N' (none)
// // int winCombo[3] = {-1, -1, -1};  // indices of the winning 3 cells
// // unsigned long winTime = 0;
// // bool roundEnded = false;

// // // Cursor position (0–8)
// // int cursorIndex = 0;

// // int screen_width, screen_height;
// // const int cell_size = 80;
// // int x_start, y_start;  // computed in setup()

// // struct Move {
// //   int index;
// //   char symbol;
// // };

// // Move moveQueue[6];  // FIFO queue of last 6 moves
// // int moveCount = 0;  // total moves placed

// // //Player score
// // int xWins = 0;
// // int oWins = 0;

// // //Background orange color
// // uint16_t orange_color = tft.color565(0xFF, 0x70, 0x00);

// // static bool buttonPreviouslyPressed = false;
// // int selection = 0;
// // int subselection = 0;
// // const unsigned long moveDelay = 100;

// // NumPad pad(tft, 
// //           drawing,
// //           up, 
// //           down, 
// //           left, 
// //           right, 
// //           A);

// // //Gamestate
// // typedef enum state{ 
// //             HOMESCREEN, 
// //             MULTIPLAYER, 
// //             SINGLE_PLAYER,
// //             GAMEOVER_SCREEN,
// //             BLUETOOTH_NUMPAD,
// //             MULTIPLAYER_SELECTION,
// //             JOIN_SCREEN,
// //           }State;

// // State game_state = HOMESCREEN;

// // void setup() {
// //   // Initialize display
// //   tft.init();
// //   tft.setRotation(3);

// //   tft.fillScreen(orange_color);

// //   // Get screen dimensions dynamically
// //   screen_width = tft.width();    // e.g., 240 or 320
// //   screen_height = tft.height();  // e.g., 320

// //   if (!SD.begin(SD_CS)) {
// //     Serial.println("Card Mount Failed");
// //     return;
// //   }
// //   uint8_t cardType = SD.cardType();

// //   if (cardType == CARD_NONE) {
// //     Serial.println("No SD card attached");
// //     return;
// //   }

// //   // Compute board position to center it
// //   JpegDrawing::ImageInfo dim = drawing.getJpegDimensions(BOARD_PATH); 
// //   x_start = (SCREEN_WIDTH - dim.width) / 2;
// //   y_start = (SCREEN_HEIGHT - dim.height) / 2;

// //   drawHomeScreen();
// // }

// // void loop() {
// //   static int lastCursor = -1;
// //   static unsigned long lastMoveTime = 0;

// //   if(game_state == HOMESCREEN){
// //     if(millis() - lastMoveTime > moveDelay/2){
// //       if(A.wasJustPressed()){
// //         if(selection == 0){
// //           game_state = SINGLE_PLAYER;
// //           // Clear the screen with orange background
// //           tft.fillScreen(orange_color);
// //           // Draw initial screen
// //           drawAllPlaying();
// //         }else if(selection == 1){
// //           game_state = MULTIPLAYER_SELECTION;
// //           drawHomescreenSelect();
// //         }
// //       }
// //       // Selection logic
// //       if(up.isPressed()){
// //         selection = 0;
// //         drawHomescreenSelect();
// //       }
// //       else if(down.isPressed()){
// //         selection = 1;
// //         drawHomescreenSelect();
// //       }
// //       lastMoveTime = millis();
// //     }
// //   }
// //   else if(game_state == MULTIPLAYER_SELECTION){
// //     if (!roundEnded && millis() - lastMoveTime > moveDelay) {
// //       if(A.wasJustPressed()){
// //         if(subselection == 0){
// //           game_state = JOIN_SCREEN;
// //           tft.fillScreen(TFT_BLUE);
// //         }else{
// //           game_state = BLUETOOTH_NUMPAD;
// //           pad.numPadSetup();
// //         }
// //       }
// //       if(up.isPressed()){
// //         game_state = HOMESCREEN;
// //         drawHomescreenSelect();
// //       }
// //       else if(left.isPressed()){
// //         if(subselection == 1){
// //           subselection = 0;
// //           drawHomescreenSelect();
// //         }
// //       }
// //       else if(right.isPressed()){
// //         if(subselection == 0){
// //           subselection = 1;
// //           drawHomescreenSelect();
// //         }
// //       }
// //       lastMoveTime = millis();
// //     }
// //   }
// //   else if(game_state == SINGLE_PLAYER){
// //     if (!roundEnded && millis() - lastMoveTime > moveDelay/2) {
// //       if (up.isPressed() && cursorIndex >= 3) {
// //         cursorIndex -= 3;
// //         lastMoveTime = millis();
// //       } else if (down.isPressed() && cursorIndex <= 5) {
// //         cursorIndex += 3;
// //         lastMoveTime = millis();
// //       }
// //        else if (left.isPressed() && cursorIndex % 3 != 0) {
// //         cursorIndex -= 1;
// //         lastMoveTime = millis();
// //       } 
// //       else if (right.isPressed() && cursorIndex % 3 != 2) {
// //         cursorIndex += 1;
// //         lastMoveTime = millis();
// //       }
// //     }

// //     // Piece Placement
// //     bool selectPressed = A.wasJustPressed();

// //     if (!roundEnded && selectPressed && !buttonPreviouslyPressed && board[cursorIndex] == "**") {
// //       if (moveCount >= 6) {
// //         int oldIndex = moveQueue[0].index;
// //         board[oldIndex] = "**";
// //         for (int i = 1; i < 6; i++) moveQueue[i - 1] = moveQueue[i];
// //         moveCount = 5;
// //       }

// //       board[cursorIndex] = String(currentPlayer);
// //       moveQueue[moveCount].index = cursorIndex;
// //       moveQueue[moveCount].symbol = currentPlayer;
// //       moveCount++;

// //       currentPlayer = (currentPlayer == 'X') ? 'O' : 'X';
// //       checkWinner();
// //     }

// //     buttonPreviouslyPressed = selectPressed;

// //     // Redraw
// //     if (cursorIndex != lastCursor || selectPressed) {
// //       drawAllPlaying();
// //       drawWinLine();
// //       if (roundEnded) drawWinnerMessage();
// //       lastCursor = cursorIndex;
// //     }
// //     // Auto Restart
// //     if (roundEnded && millis() - winTime >= 5000 && xWins < 2 && oWins < 2) {
// //       for (int i = 0; i < 9; i++) board[i] = "**";
// //       currentPlayer = 'X';
// //       cursorIndex = 0;
// //       lastCursor = -1;
// //       winner = 'N';
// //       winCombo[0] = winCombo[1] = winCombo[2] = -1;
// //       roundEnded = false;
// //       moveCount = 0;
// //       tft.fillScreen(orange_color);
// //       drawAllPlaying();
// //     }
// //     else if(xWins >= 2 || oWins >= 2){
// //       game_state = GAMEOVER_SCREEN;
// //       for (int i = 0; i < 9; i++) board[i] = "**";
// //       currentPlayer = 'X';
// //       cursorIndex = 0;
// //       lastCursor = -1;
// //       winner = 'N';
// //       winCombo[0] = winCombo[1] = winCombo[2] = -1;
// //       roundEnded = false;
// //       moveCount = 0;
// //       // Clear the screen with orange background
// //       tft.fillScreen(orange_color);
// //     }
// //   }
// //   else if(game_state == GAMEOVER_SCREEN){
// //     drawEndScreen();
// //     if(millis() - lastMoveTime > moveDelay){
// //       if(A.wasJustPressed()){
// //         game_state = HOMESCREEN;
// //         oWins = 0;
// //         xWins = 0;

// //         // Draw homescreen
// //         drawHomeScreen();
// //       }
// //       lastMoveTime = millis();
// //     }
// //   }
// //   else if(game_state == BLUETOOTH_NUMPAD){
// //     pad.handleButtonInput(&lastMoveTime,  moveDelay);
// //   }
// // }

// // void checkWinner() {
// //   const int wins[8][3] = {
// //     {0, 1, 2}, {3, 4, 5}, {6, 7, 8}, // rows
// //     {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, // cols
// //     {0, 4, 8}, {2, 4, 6}             // diagonals
// //   };

// //   for (int i = 0; i < 8; i++) {
// //     String a = board[wins[i][0]];
// //     String b = board[wins[i][1]];
// //     String c = board[wins[i][2]];

// //     if (a != "**" && a == b && b == c) {
// //       winner = a.charAt(0);
// //       winCombo[0] = wins[i][0];
// //       winCombo[1] = wins[i][1];
// //       winCombo[2] = wins[i][2];
// //       winTime = millis();
// //       roundEnded = true;

// //       if (winner == 'X') xWins++;
// //       else if (winner == 'O') oWins++;

// //       return;
// //     }
// //   }

// //   // Check draw
// //   bool full = true;
// //   for (int i = 0; i < 9; i++) {
// //     if (board[i] == "**") {
// //       full = false;
// //       break;
// //     }
// //   }
// //   if (full) {
// //     winner = 'D';
// //     winTime = millis();
// //     roundEnded = true;
// //   }
// // }

// // ///////// DRAWING //////////////////

// // void drawEndScreen() {
  
// //   // Set text properties
// //   tft.setTextDatum(MC_DATUM);
// //   tft.setTextColor(TFT_WHITE);
// //   tft.setTextSize(4);
  
// //   // Display winner
// //   String winnerText = (xWins > oWins) ? "X WINS THE GAME!" : "O WINS THE GAME!";
// //   tft.drawString(winnerText, SCREEN_WIDTH / 2, 80);
  
// //   // Display final score
// //   tft.setTextSize(3);
// //   tft.drawString("FINAL SCORE", SCREEN_WIDTH / 2, 130);
  
// //   // Draw score display
// //   int scoreYPos = 180;
// //   int xPos1 = SCREEN_WIDTH / 2 - 80;
// //   int xPos2 = SCREEN_WIDTH / 2 + 80;
  
// //   // X score
// //   tft.setTextSize(5);
// //   tft.drawString("X", xPos1, scoreYPos);
// //   tft.drawString(String(xWins), xPos1, scoreYPos + 50);
  
// //   // Divider
// //   tft.drawString("-", SCREEN_WIDTH / 2, scoreYPos);
  
// //   // O score
// //   tft.drawString("O", xPos2, scoreYPos);
// //   tft.drawString(String(oWins), xPos2, scoreYPos + 50);
  
// //   // Instructions to continue
// //   tft.setTextSize(2);
// //   tft.drawString("Press SELECT to return to menu", SCREEN_WIDTH / 2, 280);
  
// //   // Draw trophy next to winner's symbol
// //   if (xWins > oWins) {
// //     // Draw small graphic for winner (could use a custom trophy JPG if available)
// //     tft.fillTriangle(xPos1 - 40, scoreYPos, xPos1 - 60, scoreYPos + 20, xPos1 - 20, scoreYPos + 20, TFT_YELLOW);
// //   } else {
// //     // Draw small graphic for winner
// //     tft.fillTriangle(xPos2 + 40, scoreYPos, xPos2 + 60, scoreYPos + 20, xPos2 + 20, scoreYPos + 20, TFT_YELLOW);
// //   }
// // }


// // void drawHomeScreen() {
// //   // Clear the screen with orange background
// //   tft.fillScreen(orange_color);
  
// //   // Set text properties for title
// //   tft.setTextDatum(MC_DATUM);
// //   tft.setTextColor(TFT_WHITE);
// //   tft.setTextSize(4);
  
// //   // Draw game title
// //   tft.drawString("TIC TAC TOE", SCREEN_WIDTH / 2, 40);
  
// //   // Draw tic-tac-toe grid manually in center
// //   int gridSize = 90; // Total grid size
// //   int cellSize = gridSize / 3;
// //   int gridX = (SCREEN_WIDTH - gridSize) / 2;
// //   int gridY = 30;
  
// //   // Draw the grid lines with thickness of 5 pixels
// //   // Vertical lines
// //   tft.fillRect(gridX + cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
// //   tft.fillRect(gridX + 2*cellSize - 2, gridY + 50, 5, gridSize, TFT_WHITE);
  
// //   // Horizontal lines
// //   tft.fillRect(gridX, gridY + cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
// //   tft.fillRect(gridX, gridY + 2*cellSize - 2 + 50, gridSize, 5, TFT_WHITE);
  
// //   // Display instructions
// //   tft.setTextSize(2);
// //   tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, 200);
// //   tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, 250);

// //   drawHomescreenSelect();
// // }

// // void drawHomescreenSelect() {
// //   int y_single = 200;
// //   int y_multi = 250;
// //   int y_sub1 = y_multi + 20;
// //   int y_sub2 = y_multi + 40;

// //   // Clear areas
// //   tft.setTextDatum(MC_DATUM);
// //   tft.setTextColor(TFT_WHITE);
// //   tft.setTextSize(1);  // reset default

// //   tft.fillRect(0, y_single - 15, SCREEN_WIDTH, 35, orange_color);
// //   tft.fillRect(0, y_multi - 15, SCREEN_WIDTH, 80, orange_color);  // extra space for sub-options

// //   if (selection == 0) {
// //     // Single-player selected
// //     tft.setTextSize(3);
// //     tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

// //     tft.setTextSize(2);
// //     tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);
// //   } else {
// //     // Multiplayer selected
// //     tft.setTextSize(2);
// //     tft.drawString("Press for Single-Player", SCREEN_WIDTH / 2, y_single);

// //     tft.setTextSize(3);
// //     tft.drawString("Press for Multiplayer", SCREEN_WIDTH / 2, y_multi);

// //     // Draw sub-options
// //     tft.setTextSize(1);
// //     const char* sub1 = "Press to Host a Game";
// //     const char* sub2 = "Press to Join a Game";

// //     int sub1Width = tft.textWidth(sub1);
// //     int sub2Width = tft.textWidth(sub2);

// //     // Draw highlight rectangles for selected sub-option
// //     if (game_state == MULTIPLAYER_SELECTION) {
// //       const char* sub1 = "Host a Game";
// //       const char* sub2 = "Join a Game";

// //       int textSize = 2;
// //       tft.setTextSize(textSize);
// //       tft.setTextDatum(MC_DATUM);

// //       int y_sub = y_multi + 40;  // vertical position for both buttons
// //       int padding_x = 10;        // horizontal padding around text
// //       int padding_y = 2;         // vertical padding around text

// //       int sub1Width = tft.textWidth(sub1);
// //       int sub2Width = tft.textWidth(sub2);

// //       int sub1BoxWidth = sub1Width + padding_x * 2;
// //       int sub2BoxWidth = sub2Width + padding_x * 2;
// //       int boxHeight = 16 * textSize + padding_y * 2;

// //       int x_sub1 = SCREEN_WIDTH / 4;
// //       int x_sub2 = 3 * SCREEN_WIDTH / 4;

// //       // Draw highlight rectangle if selected
// //       if (subselection == 0) {
// //         tft.drawRect(x_sub1 - sub1BoxWidth / 2, y_sub - boxHeight / 2, sub1BoxWidth, boxHeight, TFT_WHITE);
// //       } else if (subselection == 1) {
// //         tft.drawRect(x_sub2 - sub2BoxWidth / 2, y_sub - boxHeight / 2, sub2BoxWidth, boxHeight, TFT_WHITE);
// //       }

// //       // Draw the sub-option text
// //       tft.drawString(sub1, x_sub1, y_sub);
// //       tft.drawString(sub2, x_sub2, y_sub);
// //     }
// //   }
// // }


// // void drawAllPlaying(){
// //   drawScoreboard();
// //   drawGrid();
// //   drawing.pushSprite();
// //   highlightCursor(cursorIndex);
// // }

// // void drawGrid() {

// //   drawing.drawSdJpeg(BOARD_PATH, x_start, y_start);
  
// //   // Draw current board state
// //   for (int i = 0; i < 9; i++) {
// //     if (board[i] != "**") {
// //       int row = i / 3;
// //       int col = i % 3;
// //       int x = col * cell_size + cell_size/3 - 3;
// //       int y = row * cell_size + cell_size/3 - 3;

// //       if(board[i] == "X"){
// //         if(moveCount >= 5 && moveQueue[0].index == i){
// //           drawing.drawSdJpeg(DIS_X_PATH, x, y);
// //         }else{
// //           drawing.drawSdJpeg(X_PATH, x, y);
// //         }
// //       }else{
// //         if(moveCount >= 5 && moveQueue[0].index == i){
// //           drawing.drawSdJpeg(DIS_O_PATH, x, y);
// //         }else{
// //           drawing.drawSdJpeg(O_PATH, x, y);
// //         }
// //       }
// //     }
// //   }
// // }

// // void highlightCursor(int index) {
// //   int row = index / 3;
// //   int col = index % 3;

// //   int x = x_start + col * cell_size + cell_size/3 - 3;
// //   int y = y_start + row * cell_size + cell_size/3 - 3;

// //   tft.drawRect(x, y, cell_size - 30, cell_size - 30, TFT_WHITE);
// // }

// // void clearCursor(int index) {
// //   if (index < 0 || index > 8) return;

// //   int row = index / 3;
// //   int col = index % 3;

// //   int x = x_start + col * cell_size;
// //   int y = y_start + row * cell_size;

// //   // Fully clear and redraw the tile as white
// //   tft.fillRect(x, y, cell_size, cell_size, TFT_WHITE);
// //   tft.drawRect(x, y, cell_size, cell_size, TFT_BLACK);

// //   // Redraw the grid lines manually if needed (for crossovers)
// //   if (col > 0) tft.drawLine(x, y, x, y + cell_size, TFT_BLACK); // left
// //   if (col < 2) tft.drawLine(x + cell_size, y, x + cell_size, y + cell_size, TFT_BLACK); // right
// //   if (row > 0) tft.drawLine(x, y, x + cell_size, y, TFT_BLACK); // top
// //   if (row < 2) tft.drawLine(x, y + cell_size, x + cell_size, y + cell_size, TFT_BLACK); // bottom
// // }

// // void drawWinLine() {
// //   if (winner != 'X' && winner != 'O') return;

// //   int i1 = winCombo[0];
// //   int i3 = winCombo[2];

// //   int row1 = i1 / 3, col1 = i1 % 3;
// //   int row3 = i3 / 3, col3 = i3 % 3;

// //   int x1 = x_start + col1 * cell_size + cell_size / 2;
// //   int y1 = y_start + row1 * cell_size + cell_size / 2;
// //   int x3 = x_start + col3 * cell_size + cell_size / 2;
// //   int y3 = y_start + row3 * cell_size + cell_size / 2;

// //   uint16_t color = (winner == 'X') ? TFT_RED : TFT_BLUE;

// //   for (int offset = -2; offset <= 2; offset++) {
// //     tft.drawLine(x1 + offset, y1, x3 + offset, y3, color);
// //     tft.drawLine(x1, y1 + offset, x3, y3 + offset, color);
// //   }
// // }

// // void drawWinnerMessage() {
// //   tft.setTextDatum(MC_DATUM);
// //   tft.setTextSize(3);
// //   tft.setTextColor(TFT_BLACK, TFT_WHITE);

// //   String msg = "";
// //   if (winner == 'X') msg = "X Wins!";
// //   else if (winner == 'O') msg = "O Wins!";
// //   else if (winner == 'D') msg = "Draw!";

// //   tft.drawString(msg, tft.width() / 2, 30);  // above grid
// // }

// // void drawScoreboard() {
// //   int centerY = tft.height() / 2;
// //   int padding = 20;

// //   // Settings for big scoreboard
// //   int textSize = 4;
// //   tft.setTextSize(textSize);
// //   tft.setTextDatum(MC_DATUM);

// //   int underlineWidth = 40;
// //   int underlineThickness = 4;   // <== THICKNESS OF THE LINE
// //   int underlineOffset = 24;     // Vertical distance from text to line
// //   int scoreOffset = 32;         // Distance from underline to score

// //   // === X Side ===
// //   tft.setTextColor(TFT_WHITE, orange_color);
// //   int xX = padding + underlineWidth;
// //   int yX = centerY - (underlineOffset + scoreOffset) / 2;
// //   tft.drawString("X", xX, yX);

// //   // Thick underline using fillRect
// //   int xLineY = yX + underlineOffset;
// //   tft.fillRect(xX - underlineWidth / 2, xLineY, underlineWidth, underlineThickness, TFT_WHITE);

// //   // Score for X
// //   tft.drawString(String(xWins), xX, xLineY + scoreOffset);

// //   // === O Side ===
// //   tft.setTextColor(TFT_WHITE, orange_color);
// //   int xO = tft.width() - padding - underlineWidth;
// //   int yO = yX;
// //   tft.drawString("O", xO, yO);

// //   // Thick underline using fillRect
// //   int oLineY = yO + underlineOffset;
// //   tft.fillRect(xO - underlineWidth / 2, oLineY, underlineWidth, underlineThickness, TFT_WHITE);

// //   // Score for O
// //   tft.drawString(String(oWins), xO, oLineY + scoreOffset);
// // }

// #include <TFT_eSPI.h>
// TFT_eSPI tft = TFT_eSPI();
// char name[6] = "     ";
// char prevChars[6] = "     ";

// uint16_t gray = tft.color565(80, 80, 80);

// enum SettingFocus {
//   FOCUS_VOLUME,
//   FOCUS_NAME,
//   FOCUS_COLOR_INVERTED,
//   FOCUS_BUTTONS, // "Back" and "Save" share one row
// };

// bool colorInverted = false;

// #define SCREEN_WIDTH  480
// #define SCREEN_HEIGHT 320

// int currentFocus = FOCUS_VOLUME;
// bool onSaveButton = false;  // false = "Back", true = "Save"

// // Mock values
// int brightness = 60; // percentage
// int volume = 50;     // percentage
// int selectedLetterIndex = 0;
// int prevSelectedLetterIndex = selectedLetterIndex;

// bool editingName = false;

// void drawCheckbox(int y, const char* label, bool checked, bool selected) {
//   tft.setTextDatum(TL_DATUM);
//   tft.setTextSize(2);
//   tft.setTextColor(TFT_WHITE, gray);
//   tft.drawString(label, 20, y);

//   int boxSize = 20;
//   int x = 200 + 5;

//   tft.drawRect(x, y, boxSize, boxSize, TFT_WHITE);
//   tft.drawRect(x - 2, y - 2, boxSize + 4, boxSize + 4, selected ? TFT_WHITE : gray);

//   tft.drawLine(x + 4, y + 10, x + 8, y + 14, checked ? TFT_WHITE : gray);
//   tft.drawLine(x + 8, y + 14, x + 16, y + 4, checked ? TFT_WHITE : gray);

// }

// void drawSlider(int y, const char* label, int valuePercent, bool selected) {
//   tft.setTextDatum(TL_DATUM);
//   tft.setTextColor(TFT_WHITE, gray);
//   tft.drawString(label, 20, y);

//   // Underline if selected
//   int labelWidth = tft.textWidth(label);
//   tft.drawLine(20, y + 18, 20 + labelWidth, y + 18, selected ? TFT_WHITE : gray);

//   int sliderX = 170;
//   int sliderY = y;
//   int sliderW = 200;
//   int sliderH = 15;

//   // Calculate fill width
//   int fillW = (valuePercent * sliderW) / 100;

//   // Clear the empty part of the slider (to the right of the fill)
//   int emptyX = sliderX + fillW;
//   int emptyWidth = sliderW - fillW;
//   if (emptyWidth > 0) {
//     tft.fillRect(emptyX, sliderY + 1, emptyWidth, sliderH - 2, gray);
//   }

//   // Draw filled part
//   if (fillW > 2) {
//     tft.fillRect(sliderX + 1, sliderY + 1, fillW - 2, sliderH - 2, TFT_WHITE);
//   }

//     // Draw slider border
//   tft.drawRect(sliderX, sliderY, sliderW, sliderH, TFT_WHITE);

//   // Clear and redraw percentage text
//   int percentX = sliderX + sliderW + 10;
//   tft.fillRect(percentX, y, 50, 16, gray);
//   char buf[8];
//   sprintf(buf, "%d%%", valuePercent);
//   tft.drawString(buf, percentX, y);
// }

// void drawNameField(int y, char name[], bool selected) {
//   tft.setTextDatum(TL_DATUM);
//   tft.setTextSize(2);
//   tft.drawString("Name:", 20, y);

//   int labelWidth = tft.textWidth("Name:");
//   tft.drawLine(20, y + 18, 20 + labelWidth, y + 18, selected ? TFT_WHITE : gray);

//   int boxSize = 40;
//   int spacing = 10;
//   int startX = 150;

//   for (int i = 0; i < 5; ++i) {
//     int x = startX + i * (boxSize + spacing);

//     if(i == prevSelectedLetterIndex && prevSelectedLetterIndex != selectedLetterIndex){
//       // Remove highlight
//       tft.drawRect(x - 2, y - 2, boxSize + 4, boxSize + 4, gray);

//       //remove arrows
//       tft.fillRect(x + boxSize / 2 - 6, y - 16, 12, 14, gray);       // up
//       tft.fillRect(x + boxSize / 2 - 6, y + boxSize + 4, 12, 14, gray); // down

//       prevSelectedLetterIndex = selectedLetterIndex;
//     }

//     if (i == selectedLetterIndex) {
//       // Draw highlight
//       tft.drawRect(x - 2, y - 2, boxSize + 4, boxSize + 4, currentFocus == FOCUS_NAME ? TFT_WHITE : gray);

//       if (editingName) {

//         int base = 10;
//         int height = 8;
        
//         // Up arrow
//         tft.fillTriangle(x + boxSize / 2, y - height - 4, 
//                           x + boxSize / 2 - base / 2, y - 4, 
//                           x + boxSize / 2 + base / 2, y - 4, TFT_WHITE);
//         // Down arrow
//         tft.fillTriangle(x + boxSize / 2, y + boxSize + height + 4, 
//                           x + boxSize / 2 - base / 2, y + boxSize + 4,
//                           x + boxSize / 2 + base / 2, y + boxSize + 4, TFT_WHITE);
//       } else {
//         // Clear arrows
//         tft.fillRect(x + boxSize / 2 - 6, y - 16, 12, 14, gray);       // up
//         tft.fillRect(x + boxSize / 2 - 6, y + boxSize + 4, 12, 14, gray); // down
//       }
//     }

//     // Box
//     tft.drawRect(x, y, boxSize, boxSize, TFT_WHITE);

//     if(prevChars[i] != name[i]){
//       tft.fillRect(x + 1, y + 1, boxSize - 2, boxSize - 2, gray);
//       prevChars[i] = name[i];
//     }
//     // Letter
//     char str[2] = {name[i], '\0'};
//     tft.setTextDatum(MC_DATUM);
//     tft.drawString(str, x + boxSize / 2, y + boxSize / 2, 2);
//   }
// }

// void drawButton(const char* label, int x, int y, int w, int h, bool selected) {
//   tft.drawRect(x, y, w, h, TFT_WHITE);
//   tft.setTextDatum(MC_DATUM);
//   tft.drawString(label, x + w / 2, y + h / 2);

//   // Add outer rectangle to indicate selection or gray to show it's not selected
//   tft.drawRect(x - 2, y - 2, w + 4, h + 4, selected ? TFT_WHITE : gray);
// }

// void drawSettingsScreen() {

//   // Title
//   tft.setTextDatum(MC_DATUM);
//   tft.setTextColor(TFT_WHITE, gray);
//   String title = "SETTINGS";
//   tft.setTextSize(5);
//   int titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2;
//   int titleY = 10;
//   tft.setTextColor(TFT_WHITE);
//   tft.setCursor(titleX, titleY);
//   tft.print(title);

//   //Sliders
//   tft.setTextSize(2);
//   // drawSlider(70, "Brightness:", brightness, currentFocus == FOCUS_BRIGHTNESS);
//   drawSlider(100, "Volume:", volume, currentFocus == FOCUS_VOLUME);
  
//   //Name
//   drawNameField(150, name, currentFocus == FOCUS_NAME);

//   drawCheckbox(220, "Color Inverted:", colorInverted, currentFocus == FOCUS_COLOR_INVERTED);

//   drawButton("Back", 80, 250, 100, 40, currentFocus == FOCUS_BUTTONS && !onSaveButton);
//   drawButton("Save", SCREEN_WIDTH - 180, 250, 100, 40, currentFocus == FOCUS_BUTTONS && onSaveButton);
// }

// void setup() {
//   tft.init();
//   tft.setRotation(3); // Landscape
//   tft.fillScreen(gray);
//   drawSettingsScreen();
// }
// static unsigned long lastMoveTime = 0;
// const unsigned long moveDelay = 100;

// void loop() {

//   if(millis() - lastMoveTime > moveDelay){
//   bool screenNeedsUpdate = false;

//   if (down.wasJustPressed()) {
//     if (editingName && currentFocus == FOCUS_NAME) {
//       char& c = name[selectedLetterIndex];
//         c = (c > 'A' && c <= 'Z') ? c - 1 : 'Z';
//         screenNeedsUpdate = true;
//     }
//     else if (currentFocus < FOCUS_BUTTONS) {
//       currentFocus++;
//       screenNeedsUpdate = true;
//     }
//   }

//   if (up.wasJustPressed()) {
//     if(editingName && currentFocus == FOCUS_NAME){
//       char& c = name[selectedLetterIndex];
//       c = (c >= 'A' && c < 'Z') ? c + 1 : 'A';
//       screenNeedsUpdate = true;
//     }
//     else if (currentFocus > FOCUS_VOLUME) {
//       currentFocus--;
//       screenNeedsUpdate = true;
//     }
//   }

//   if (left.wasJustPressed()) {
//     if (currentFocus == FOCUS_BUTTONS) {
//       onSaveButton = false;
//       screenNeedsUpdate = true;
//     } else if (currentFocus == FOCUS_NAME && selectedLetterIndex > 0) {
//       selectedLetterIndex--;
//       screenNeedsUpdate = true;
//     } else if (currentFocus == FOCUS_VOLUME && volume > 0) {
//       volume -= 5;
//       screenNeedsUpdate = true;
//     }
//   }

//   if (right.wasJustPressed()) {
//     if (currentFocus == FOCUS_BUTTONS) {
//       onSaveButton = true;
//       screenNeedsUpdate = true;
//     } else if (currentFocus == FOCUS_NAME && selectedLetterIndex < 4) {
//       selectedLetterIndex++;
//       screenNeedsUpdate = true;
//     } else if (currentFocus == FOCUS_VOLUME && volume < 100) {
//       volume += 5;
//       screenNeedsUpdate = true;
//     }
//   }

//   if (A.wasJustPressed()) {
//     if (currentFocus == FOCUS_NAME) {
//       editingName = !editingName;
//       screenNeedsUpdate = true;
//     } else if (currentFocus == FOCUS_BUTTONS) {
//       if (onSaveButton) {
//         tft.fillScreen(TFT_GREEN); // simulate save
//       } else {
//         tft.fillScreen(TFT_RED); // simulate back
//       }
//     }else if (currentFocus == FOCUS_COLOR_INVERTED) {
//       colorInverted = !colorInverted;
//       screenNeedsUpdate = true;
//     }
//   }

//   if (screenNeedsUpdate)
//     drawSettingsScreen();
//   lastMoveTime = millis();
//   }
// }

#include <Arduino.h>

// Pin definition
const int analogPin = 36; // GPIO36 is also known as VP

void setup() {
  Serial.begin(115200); // Start the serial monitor
  analogReadResolution(12); // Optional: Set ADC resolution to 12 bits (0–4095)
}

void loop() {
  int analogValue = analogRead(analogPin); // Read the analog value
  Serial.print("Analog value on GPIO36: ");
  Serial.println(analogValue); // Print to Serial Monitor
  delay(500); // Wait for half a second
}