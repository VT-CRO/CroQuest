#include "main.hpp"
#include "pong.h"

void drawPaddle(LGFX& tft, Paddle paddle, Prev_Paddle* prev_paddle);
void drawBall(LGFX& tft, Ball* ball, Ball* prev_ball);
void drawScore(LGFX& tft, int score0, int score1);
void draw_homescreen(LGFX& tft, bool *first_home_draw);
void draw_endscreen(LGFX& tft, int score0, int score1);