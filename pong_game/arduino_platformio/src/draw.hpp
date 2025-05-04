#include "main.hpp"
#include "pong.h"

#define NUM_PAD_WIDTH 3
#define NUM_PAD_LENGTH 4

void drawPaddle(TFT_eSPI& tft, Paddle paddle);
void drawBall(TFT_eSPI& tft, Ball* ball);
void drawScore(TFT_eSPI& tft, int score0, int score1);

void erasePaddle(TFT_eSPI& tft, Paddle paddle);
void eraseBall(TFT_eSPI& tft, Ball* ball);

void draw_homescreen(TFT_eSPI& tft, bool *first_home_draw);
void draw_endscreen(TFT_eSPI& tft, int score0, int score1);
void draw_multiplayer_screen(TFT_eSPI& tft);
void init_buttons();
void draw_all_buttons(TFT_eSPI& tft);
void draw_button_hover(TFT_eSPI& tft, int col, int row);
int draw_button_pressed(TFT_eSPI& tft, int col, int row);
void draw_button_default(TFT_eSPI& tft, int col, int row);
void draw_numbers(TFT_eSPI& tft, String code);
void draw_waiting_screen(TFT_eSPI& tft, bool host_ready, bool guest_ready, String host_id);
void draw_back_waiting(TFT_eSPI& tft, int waiting_select);
void draw_back_multiplayer(TFT_eSPI& tft, int multiplayer_select);
void draw_client_input_back(TFT_eSPI& tft, int client_select);
void erase_score(TFT_eSPI& tft);