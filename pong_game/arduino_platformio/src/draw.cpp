#include "draw.hpp"

// Function to draw the paddle on the screen
void drawPaddle(LGFX& tft, Paddle paddle, Prev_Paddle* prev_paddle) {
// Clear previous paddle position (if any)
    if(prev_paddle->y != paddle.y || prev_paddle->paddle_mod){
        tft.fillRect(paddle.x, prev_paddle->y, paddle.w, paddle.h, TFT_BLACK);
        prev_paddle->y = paddle.y;
        prev_paddle->paddle_mod = false;
    }

    // Draw new paddle position
    tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_WHITE);
}

// Function to draw the ball on the screen
void drawBall(LGFX& tft, Ball* ball, Ball* prev_ball) {
    if(prev_ball->x != ball->x || prev_ball->y != ball->y){
        tft.fillCircle(prev_ball->x + (ball->w/2), prev_ball->y + (ball->w/2), ball->w/2, TFT_BLACK);
        prev_ball->x = ball->x;
        prev_ball->y = ball->y;
    }

    // Draw new ball position
    tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_WHITE);
}

// Function to draw the score on the screen
void drawScore(LGFX& tft, int score0, int score1) {
    tft.setTextSize(2);

    tft.drawLine(SCREEN_WIDTH / 2, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, TFT_WHITE);

    // Create score strings
    String scoreText1 = "P1: " + String(score0);
    String scoreText2 = "P2: " + String(score1);

    // Get text widths
    int width1 = tft.textWidth(scoreText1);
    int width2 = tft.textWidth(scoreText2);

    // Calculate positions so each string is centered in its half
    int leftX = (SCREEN_WIDTH / 2 - width1) / 2;
    int rightX = SCREEN_WIDTH / 2 + (SCREEN_WIDTH / 2 - width2) / 2;

    // Draw the text
    tft.setCursor(leftX, 5);
    tft.print(scoreText1);

    tft.setCursor(rightX, 5);
    tft.print(scoreText2);
}

//Function to draw homescreen bitmap onto the screen
void draw_homescreen(LGFX& tft, bool * first_home_draw)
{
    // Check if the BMP file exists on the SD card
    if(*first_home_draw){
        if (SD.exists("/home_screen.bmp")) {
        Serial.println("Drawing BMP...");
        tft.drawBmpFile(SD, "/home_screen.bmp", 0, 0);
        } else {
        Serial.println("File not found!");
        }
        *first_home_draw = false;
    }
}

//Draws the end screen
void draw_endscreen(LGFX& tft, int score0, int score1){
    // Player that won the game
    tft.setTextSize(3);
    String player = score0 >= GAME_WON ? "Player 1 Wins!" : "Player 2 Wins!";
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(player)) / 2, 60);
    tft.print(player);

    // The score for both players
    tft.setTextSize(2);
    String scorestr0 = "P1: " + String(score0) + "  ";
    String scorestr1 = "P2: " + String(score1);
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(scorestr0) - tft.textWidth(scorestr1)) / 2, 120);
    tft.print(scorestr0);
    tft.print(scorestr1);

    // Reset instruction
    String restart = "Press (A) to restart";
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(restart)) / 2, 180);
    tft.print(restart);

    // Home screen instruction
    String home = "Press (B) for home screen";
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(home)) / 2, 180 + tft.fontHeight() + 5);
    tft.print(home);
}