#include "main.hpp"
#include "pong.h"

#define NUM_PAD_WIDTH 3
#define NUM_PAD_LENGTH 4

void drawPaddle(LGFX& tft, Paddle paddle, Prev_Paddle* prev_paddle);
void drawBall(LGFX& tft, Ball* ball, Ball* prev_ball);
void drawScore(LGFX& tft, int score0, int score1);
void draw_homescreen(LGFX& tft, bool *first_home_draw);
void draw_endscreen(LGFX& tft, int score0, int score1);
void draw_multiplayer_screen(LGFX& tft);
void init_buttons();
void draw_all_buttons(LGFX& tft);
void draw_button_hover(LGFX& tft, int col, int row);
int draw_button_pressed(LGFX& tft, int col, int row);
void draw_button_default(LGFX& tft, int col, int row);
void draw_numbers(LGFX& tft, String code);
void draw_waiting_screen(LGFX& tft, bool host_ready, bool guest_ready, String host_id);