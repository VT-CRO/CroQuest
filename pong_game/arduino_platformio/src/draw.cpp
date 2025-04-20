#include "draw.hpp"

#define BUTTON_WIDTH SCREEN_WIDTH/7
#define BUTTON_HEIGHT SCREEN_HEIGHT/7
#define BUTTON_SPACING 0

#define BUTTON_START_X ((SCREEN_WIDTH - (NUM_PAD_WIDTH * BUTTON_WIDTH + (NUM_PAD_WIDTH - 1) * BUTTON_SPACING)) / 2)

#define BUTTON_START_Y (SCREEN_HEIGHT * 0.7 - (NUM_PAD_LENGTH * BUTTON_HEIGHT + (NUM_PAD_LENGTH - 1) * BUTTON_SPACING) / 2)

struct Button {
    int x, y, w, h;
    int button;
    String base;

    String default_img;
    String hover_img;
    String pressed_img;
};

static Button pad[NUM_PAD_WIDTH][NUM_PAD_LENGTH];

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

//Drawing multiplayer screen
void draw_multiplayer_screen(LGFX& tft) {
    tft.clear(TFT_BLACK);  // Clear the screen

    // Title
    String title = "Multiplayer Setup";
    tft.setTextSize(3);
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(title)) / 2, 40);
    tft.print(title);

    // Instruction to host a game
    tft.setTextSize(2);
    String host = "Press (A) to host a game";
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(host)) / 2, 100);
    tft.print(host);

    // Instruction to join a game
    String join = "Press (B) to join a game";
    tft.setCursor((SCREEN_WIDTH - tft.textWidth(join)) / 2, 140);
    tft.print(join);
}


/////////////// Button Logic ////////////////////////

static void draw_button(LGFX& tft, Button* b, const String& state) {
    String path;
    if (state == "hover") path = b->hover_img;
    else if (state == "pressed") path = b->pressed_img;
    else path = b->default_img;

    // Draw from file at (x, y)
    if (SD.exists(path)) {
        tft.drawBmpFile(SD, path, b->x, b->y);
    } else {
        // fallback: draw placeholder
        tft.fillRoundRect(b->x, b->y, b->w, b->h, 6, TFT_DARKGREY);
        tft.setCursor(b->x + 10, b->y + 10);
        tft.setTextSize(2);
        tft.setTextColor(TFT_WHITE);
        tft.print(b->base);
    }
}

static String make_button_filename(const String& base, const String& state) {
    return "/btn_" + base + "_" + state + ".bmp";
}

void init_buttons() {
    int button_map[NUM_PAD_LENGTH][NUM_PAD_WIDTH] = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9},
        {-2, 0, -1}
    };

    for (int row = 0; row < NUM_PAD_LENGTH; row++) {
        for (int col = 0; col < NUM_PAD_WIDTH; col++) {
            Button* b = &pad[col][row];
            b->x = BUTTON_START_X + col * (BUTTON_WIDTH + BUTTON_SPACING);
            b->y = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_SPACING);
            b->w = BUTTON_WIDTH;
            b->h = BUTTON_HEIGHT;
            b->button = button_map[row][col];

            if (b->button == -1) b->base = "enter";
            else if (b->button == -2) b->base = "del";
            else b->base = String(b->button);

            b->default_img = make_button_filename(b->base, "default");
            b->hover_img = make_button_filename(b->base, "hover");
            b->pressed_img = make_button_filename(b->base, "pressed");
        }
    }
}

// Draw all buttons in default state
void draw_all_buttons(LGFX& tft) {
    for (int row = 0; row < NUM_PAD_LENGTH; row++) {
        for (int col = 0; col < NUM_PAD_WIDTH; col++) {
            draw_button(tft, &pad[col][row], "default");
        }
    }
}

// Draw a specific button in hover state
void draw_button_hover(LGFX& tft, int col, int row) {
    draw_button(tft, &pad[col][row], "hover");
}

// Draw a specific button in pressed state
int draw_button_pressed(LGFX& tft, int col, int row) {
    draw_button(tft, &pad[col][row], "pressed");
    return pad[col][row].button;
}

// Draw a specific button in the default state
void draw_button_default(LGFX& tft, int col, int row) {
    draw_button(tft, &pad[col][row], "default");
}
//Draw the code 
void draw_numbers(LGFX& tft, String code) {
    String headerText = "Join a Host Code";

    // Draw header
    tft.setTextSize(2);
    int headerX = (SCREEN_WIDTH - tft.textWidth(headerText)) / 2;
    int headerY = BUTTON_START_Y - 90;
    tft.setCursor(headerX, headerY);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Optional: overwrite header background
    tft.print(headerText);

    // Clear previous code area
    tft.setTextSize(4);
    int codeHeight = 32;  // Approximate height for textSize 4 (can adjust)
    int codeY = BUTTON_START_Y - 50;
    tft.fillRect(0, codeY, SCREEN_WIDTH, codeHeight + 10, TFT_BLACK);

    // Draw code string
    int codeX = (SCREEN_WIDTH - tft.textWidth(code)) / 2;
    tft.setCursor(codeX, codeY);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // text with background erase
    tft.print(code);
}