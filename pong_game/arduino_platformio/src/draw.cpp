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
void drawPaddle(TFT_eSPI& tft, Paddle paddle) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new paddle position
    tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_WHITE);
}

// Function to draw the ball on the screen
void drawBall(TFT_eSPI& tft, Ball* ball) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new ball position
    tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_WHITE);
}

// Function to erase the paddle on the screen
void erasePaddle(TFT_eSPI& tft, Paddle paddle) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new paddle position
    tft.fillRect(paddle.x, paddle.y, paddle.w, paddle.h, TFT_BLACK);
}

// Function to erase the ball on the screen
void eraseBall(TFT_eSPI& tft, Ball* ball) {  // Changed from LGFX_Sprite to TFT_eSprite
    // Draw new ball position
    tft.fillCircle(ball->x + (ball->w/2), ball->y + (ball->w/2), ball->w/2, TFT_BLACK);
}

// Function to draw the score on the screen
void drawScore(TFT_eSPI& tft, int score0, int score1) {  // Changed from LGFX_Sprite to TFT_eSprite
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
void draw_homescreen(TFT_eSPI& tft, bool *first_home_draw)
{
    if (*first_home_draw) {
        tft.fillScreen(TFT_BLACK);

        // Draw big "PONG" title
        String title = "PONG";
        tft.setTextSize(8);
        // Main title with depth effect
        int titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2;
        int titleY = 30;
        tft.setTextColor(TFT_WHITE);  // Main text color
        tft.setCursor(titleX, titleY);
        tft.print(title);

        // Highlights (simulate lighting)
        titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2 - 2;
        titleY = 30 - 2;
        tft.setTextColor(TFT_LIGHTGREY);  // Lighter color for highlight
        tft.setCursor(titleX, titleY);
        tft.print(title);

        // Common button setup
        int button_radius = 22;  // Made buttons bigger
        int textSizeSmall = 2;

        // Color adjustments
        uint16_t LIGHTER_RED = tft.color565(255, 120, 120);    // Brighter red tint
        uint16_t LIGHTER_BLUE = tft.color565(120, 120, 255);   // Brighter blue tint
        uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);     // Deeper shadow

        // ===== A Button ("Join Game") =====
        int pressA_y = 180;
        int pressA_x = (SCREEN_WIDTH / 2) - 110;

        tft.setTextSize(textSizeSmall);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(pressA_x, pressA_y);
        tft.print("Press");

        int a_circle_x = SCREEN_WIDTH / 2;
        int a_circle_y = pressA_y + 15;

        // Shadow
        tft.fillCircle(a_circle_x + 4, a_circle_y + 4, button_radius, DARKER_SHADOW);
        // Button
        tft.fillCircle(a_circle_x, a_circle_y, button_radius, TFT_RED);
        // Lighter highlight - slightly left and slightly lower
        tft.fillEllipse(a_circle_x - 7, a_circle_y - 7, 7, 5, LIGHTER_RED);

        // Letter "A" (black)
        tft.setTextSize(2);
        tft.setCursor(a_circle_x - 6, a_circle_y - 8);
        tft.setTextColor(TFT_BLACK);
        tft.print("A");

        // "to join game" text
        String toJoinGame_text = "to join game";
        int toJoinGame_x = (SCREEN_WIDTH / 2) + 40;
        tft.setTextSize(textSizeSmall);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(toJoinGame_x, pressA_y);
        tft.print(toJoinGame_text);

        // ===== B Button ("Single Player") =====
        int pressB_y = 250;
        int pressB_x = (SCREEN_WIDTH / 2) - 110;

        // Shifted entire line to the left
        int pressB_x_adjusted = pressB_x - 40;  // Move 40 pixels to the left for the B button line

        tft.setTextSize(textSizeSmall);
        tft.setCursor(pressB_x_adjusted, pressB_y);
        tft.print("Press");

        int b_circle_x = SCREEN_WIDTH / 2 - 45;
        int b_circle_y = pressB_y + 15;

        // Shadow
        tft.fillCircle(b_circle_x + 4, b_circle_y + 4, button_radius, DARKER_SHADOW);
        // Button
        tft.fillCircle(b_circle_x, b_circle_y, button_radius, TFT_BLUE);
        // Lighter highlight - slightly left and lower
        tft.fillEllipse(b_circle_x - 7, b_circle_y - 7, 7, 5, LIGHTER_BLUE);

        // Letter "B" (black)
        tft.setTextSize(2);
        tft.setCursor(b_circle_x - 6, b_circle_y - 8);
        tft.setTextColor(TFT_BLACK);
        tft.print("B");

        // "for single-player" text
        String forSingle_text = "for single-player";
        int forSingle_x = pressB_x_adjusted + 150;  // Adjust position for the new left-shift (more spacing)
        tft.setTextSize(textSizeSmall);
        tft.setTextColor(TFT_WHITE);
        tft.setCursor(forSingle_x, pressB_y);
        tft.print(forSingle_text);

        *first_home_draw = false;
    }
}



//Draws the end screen
void draw_endscreen(TFT_eSPI& tft, int score0, int score1) {
    // Clear screen
    tft.fillScreen(TFT_BLACK);

    // Title (Winner message) with depth effect
    String player = score0 >= GAME_WON ? "Player 1 Wins!" : "Player 2 Wins!";
    tft.setTextSize(5);  // Larger size for the winner message
    int titleX = (SCREEN_WIDTH - tft.textWidth(player)) / 2;
    int titleY = 30;

    // Shadow effect
    tft.setTextColor(TFT_DARKGREY);  // Shadow color
    tft.setCursor(titleX + 4, titleY + 4);
    tft.print(player);

    // Main title
    tft.setTextColor(TFT_WHITE);  // Main text color
    tft.setCursor(titleX, titleY);
    tft.print(player);

    // Score display with highlights and shadows
    tft.setTextSize(3);
    String scorestr0 = "P1: " + String(score0) + "  ";
    String scorestr1 = "P2: " + String(score1);
    int scoreX = (SCREEN_WIDTH - tft.textWidth(scorestr0) - tft.textWidth(scorestr1)) / 2;
    tft.setTextColor(TFT_LIGHTGREY);
    tft.setCursor(scoreX + 4, 120 + 4);  // Slight shadow offset
    tft.print(scorestr0);
    tft.print(scorestr1);

    tft.setTextColor(TFT_WHITE);  // Reset main text color
    tft.setCursor(scoreX, 120);   // Main position
    tft.print(scorestr0);
    tft.print(scorestr1);

    // Instructions for restart and home screen with buttons

    // ----- Restart Button (A) -----
    int pressA_y = 200;
    int pressA_x = (SCREEN_WIDTH / 2) - 110;

    // "Press" text
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(pressA_x, pressA_y);
    tft.print("Press");

    // A Button (circle)
    int a_circle_x = SCREEN_WIDTH / 2;
    int a_circle_y = pressA_y + 15;
    uint16_t LIGHTER_RED = tft.color565(255, 120, 120); // Brighter red
    uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);  // Darker shadow

    // Shadow effect for button
    tft.fillCircle(a_circle_x + 4, a_circle_y + 4, 22, DARKER_SHADOW);
    // Red button
    tft.fillCircle(a_circle_x, a_circle_y, 22, TFT_RED);
    // Light red highlight
    tft.fillEllipse(a_circle_x - 7, a_circle_y - 7, 7, 5, LIGHTER_RED);

    // "A" letter inside the button
    tft.setTextSize(2);
    tft.setCursor(a_circle_x - 6, a_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("A");

    // Restart instruction text
    String restartText = "to restart";
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    int restartText_x = (SCREEN_WIDTH / 2) + 40;
    tft.setCursor(restartText_x, pressA_y);
    tft.print(restartText);

    // ----- Home Button (B) -----
    int pressB_y = 260;
    int pressB_x = (SCREEN_WIDTH / 2) - 110;

    // "Press" text
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(pressB_x - 40, pressB_y);  // Left-shifted
    tft.print("Press");

    // B Button (circle)
    int b_circle_x = SCREEN_WIDTH / 2 - 45;
    int b_circle_y = pressB_y + 15;
    uint16_t LIGHTER_BLUE = tft.color565(120, 120, 255); // Brighter blue

    // Shadow effect for button
    tft.fillCircle(b_circle_x + 4, b_circle_y + 4, 22, DARKER_SHADOW);
    // Blue button
    tft.fillCircle(b_circle_x, b_circle_y, 22, TFT_BLUE);
    // Light blue highlight
    tft.fillEllipse(b_circle_x - 7, b_circle_y - 7, 7, 5, LIGHTER_BLUE);

    // "B" letter inside the button
    tft.setTextSize(2);
    tft.setCursor(b_circle_x - 6, b_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("B");

    // Home instruction text
    String homeText = "for home screen";
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    int homeText_x = pressB_x - 40 + 150; // Adjusted position for left shift
    tft.setCursor(homeText_x, pressB_y);
    tft.print(homeText);
}

//Drawing multiplayer screen
void draw_multiplayer_screen(TFT_eSPI& tft) {  
    tft.fillScreen(TFT_BLACK);  // Clear the screen

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    tft.setCursor(10, 10);
    tft.print("< Back");

    // Title (Multiplayer Setup) with depth effect
    String title = "Multiplayer Setup";
    tft.setTextSize(4);  // Increase size for the title
    int titleX = (SCREEN_WIDTH - tft.textWidth(title)) / 2;
    int titleY = 40;

    // Shadow effect for the title
    tft.setTextColor(TFT_DARKGREY);  // Shadow color
    tft.setCursor(titleX + 4, titleY + 4);
    tft.print(title);

    // Main title
    tft.setTextColor(TFT_WHITE);  // Main text color
    tft.setCursor(titleX, titleY);
    tft.print(title);

    // Instruction to host a game with buttons and depth effects

    // ----- Host Button (A) -----
    int pressA_y = 130;  // Move "Press A" lower
    int pressA_x = (SCREEN_WIDTH / 2) - 110;

    // "Press" text for A button
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(pressA_x, pressA_y);
    tft.print("Press");

    // A Button (circle)
    int a_circle_x = SCREEN_WIDTH / 2;
    int a_circle_y = pressA_y + 15;
    uint16_t LIGHTER_RED = tft.color565(255, 120, 120); // Brighter red for highlight
    uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);  // Darker shadow

    // Shadow effect for button
    tft.fillCircle(a_circle_x + 4, a_circle_y + 4, 22, DARKER_SHADOW);
    // Red button
    tft.fillCircle(a_circle_x, a_circle_y, 22, TFT_RED);
    // Light red highlight
    tft.fillEllipse(a_circle_x - 7, a_circle_y - 7, 7, 5, LIGHTER_RED);

    // "A" letter inside the button
    tft.setTextSize(2);
    tft.setCursor(a_circle_x - 6, a_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("A");

    // "to host a game" instruction text
    String hostText = "to host a game";
    int hostText_x = (SCREEN_WIDTH / 2) + 40;
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(hostText_x, pressA_y);
    tft.print(hostText);

    // ----- Join Button (B) -----
    int pressB_y = 200;  // Move "Press B" lower
    int pressB_x = (SCREEN_WIDTH / 2) - 110;

    // Left-shift the "Press" text for B button
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(pressB_x - 40, pressB_y);  // Left-shifted
    tft.print("Press");

    // B Button (circle)
    int b_circle_x = SCREEN_WIDTH / 2 - 45;
    int b_circle_y = pressB_y + 15;
    uint16_t LIGHTER_BLUE = tft.color565(120, 120, 255); // Brighter blue for highlight

    // Shadow effect for button
    tft.fillCircle(b_circle_x + 4, b_circle_y + 4, 22, DARKER_SHADOW);
    // Blue button
    tft.fillCircle(b_circle_x, b_circle_y, 22, TFT_BLUE);
    // Light blue highlight
    tft.fillEllipse(b_circle_x - 7, b_circle_y - 7, 7, 5, LIGHTER_BLUE);

    // "B" letter inside the button
    tft.setTextSize(2);
    tft.setCursor(b_circle_x - 6, b_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("B");

    // "to join a game" instruction text
    String joinText = "to join a game";
    int joinText_x = pressB_x - 40 + 150;  // Adjusted position for left shift
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.setCursor(joinText_x, pressB_y);
    tft.print(joinText);
}

void draw_back_multiplayer(TFT_eSPI& tft, int multiplayer_select) {
    // --- Draw "Back" button at top left ---
    tft.setTextSize(2);
    if (multiplayer_select == 0) {
        tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Highlighted (white background)
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    }
    tft.setCursor(10, 10);
    tft.print("< Back");

    // --- Draw "A" button (Host game) ---
    int pressA_y = 130;
    int a_circle_x = SCREEN_WIDTH / 2;
    int a_circle_y = pressA_y + 15;
    int a_radius = 22; // from your multiplayer screen code

    uint16_t LIGHTER_RED = tft.color565(255, 120, 120);
    uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);

    // Clear behind A button
    int clear_radius = a_radius + 6;
    tft.fillCircle(a_circle_x, a_circle_y, clear_radius, TFT_BLACK);

    // Shadow
    tft.fillCircle(a_circle_x + 4, a_circle_y + 4, a_radius, DARKER_SHADOW);

    // Main red button
    tft.fillCircle(a_circle_x, a_circle_y, a_radius, TFT_RED);

    // Highlight if selected
    if (multiplayer_select == 1) {
        tft.drawCircle(a_circle_x, a_circle_y, a_radius + 4, TFT_YELLOW);  // Yellow highlight ring
    }

    // Highlight spot
    tft.fillEllipse(a_circle_x - 7, a_circle_y - 7, 7, 5, LIGHTER_RED);

    // A letter
    tft.setTextSize(2);
    tft.setCursor(a_circle_x - 6, a_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("A");

    // --- Draw "B" button (Join game) ---
    int pressB_y = 200;
    int b_circle_x = SCREEN_WIDTH / 2 - 45;
    int b_circle_y = pressB_y + 15;
    int b_radius = 22;

    uint16_t LIGHTER_BLUE = tft.color565(120, 120, 255);

    // Clear behind B button
    clear_radius = b_radius + 6;
    tft.fillCircle(b_circle_x, b_circle_y, clear_radius, TFT_BLACK);

    // Shadow
    tft.fillCircle(b_circle_x + 4, b_circle_y + 4, b_radius, DARKER_SHADOW);

    // Main blue button
    tft.fillCircle(b_circle_x, b_circle_y, b_radius, TFT_BLUE);

    // Highlight if selected
    if (multiplayer_select == 2) {
        tft.drawCircle(b_circle_x, b_circle_y, b_radius + 4, TFT_YELLOW);  // Yellow highlight ring
    }

    // Highlight spot
    tft.fillEllipse(b_circle_x - 7, b_circle_y - 7, 7, 5, LIGHTER_BLUE);

    // B letter
    tft.setTextSize(2);
    tft.setCursor(b_circle_x - 6, b_circle_y - 8);
    tft.setTextColor(TFT_BLACK);
    tft.print("B");
}



void draw_waiting_screen(TFT_eSPI& tft, bool host_ready, bool guest_ready, String host_id) {  
    tft.fillScreen(TFT_BLACK);  // Clear the screen

    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    tft.setCursor(10, 10);
    tft.print("< Back");

    // Draw Host ID at top center
    tft.setTextSize(2);
    String host_id_label = "Host ID";
    int host_id_label_x = (SCREEN_WIDTH - tft.textWidth(host_id_label)) / 2;
    tft.setCursor(host_id_label_x, 30);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.print(host_id_label);

    // Draw the actual host ID value below the label
    tft.setTextSize(3);
    int host_id_x = (SCREEN_WIDTH - tft.textWidth(host_id)) / 2;
    tft.setCursor(host_id_x, 55);
    tft.setTextColor(TFT_YELLOW, TFT_BLACK);
    tft.print(host_id);

    // Host label and status - moved down
    tft.setTextSize(2);
    String host_label = "Host";
    tft.setCursor(SCREEN_WIDTH / 4 - tft.textWidth(host_label) / 2, 110);
    tft.setTextColor(TFT_WHITE);
    tft.print(host_label);

    String host_status = host_ready ? "READY" : "Waiting...";
    tft.setTextColor(host_ready ? TFT_GREEN : TFT_LIGHTGREY);
    tft.setCursor(SCREEN_WIDTH / 4 - tft.textWidth(host_status) / 2, 135);
    tft.print(host_status);

    // Guest label and status - moved down
    String guest_label = "Guest";
    tft.setCursor(3 * SCREEN_WIDTH / 4 - tft.textWidth(guest_label) / 2, 110);
    tft.setTextColor(TFT_WHITE);
    tft.print(guest_label);

    String guest_status = guest_ready ? "READY" : "Waiting...";
    tft.setTextColor(guest_ready ? TFT_GREEN : TFT_LIGHTGREY);
    tft.setCursor(3 * SCREEN_WIDTH / 4 - tft.textWidth(guest_status) / 2, 135);
    tft.print(guest_status);

    // "Press A when ready" with red A button - moved down and shadow added
    int center_y = 210;  // Adjusted the vertical position for better spacing
    tft.setTextSize(3);  // Increased text size for better visibility

    // "Press" text
    String press_text = "Press";
    int press_x = (SCREEN_WIDTH / 2) - 150;
    tft.setCursor(press_x, center_y);
    tft.setTextColor(TFT_WHITE);
    tft.print(press_text);

    // Red "A" button with shading and larger size
    int button_radius = 28;  // Increased size of the button for better visibility
    int circle_x = SCREEN_WIDTH / 2;
    int circle_y = center_y + 10;  // Adjusted for vertical alignment

    uint16_t LIGHTER_RED = tft.color565(255, 120, 120);  // Brighter red for highlight
    uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);  // Darker shadow color

    // Shadow for button
    tft.fillCircle(circle_x + 4, circle_y + 4, button_radius, DARKER_SHADOW);  // Shadow slightly offset
    tft.fillCircle(circle_x, circle_y, button_radius, TFT_RED);  // Main button color
    tft.fillEllipse(circle_x - 7, circle_y - 7, 7, 5, LIGHTER_RED);  // Light red highlight

    // Add "A" letter inside the button
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(circle_x - 8, circle_y - 8);  // Center the "A" in the button
    tft.print("A");

    // "when ready" text
    String when_text = "when ready";
    int when_x = (SCREEN_WIDTH / 2) + 40;
    tft.setTextSize(3);  // Larger text for better readability
    tft.setCursor(when_x, center_y);
    tft.setTextColor(TFT_WHITE);
    tft.print(when_text);
}

void draw_back_waiting(TFT_eSPI& tft, int waiting_select) {
    // --- Draw "Back" button ---
    tft.setTextSize(2);
    if (waiting_select == 0) {
        tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Highlighted (white background)
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    }
    tft.setCursor(10, 10);
    tft.print("< Back");

    // --- Draw "A" button ---
    int center_y = 210;
    int circle_x = SCREEN_WIDTH / 2;
    int circle_y = center_y + 10;
    int button_radius = 28;

    uint16_t LIGHTER_RED = tft.color565(255, 120, 120);
    uint16_t DARKER_SHADOW = tft.color565(20, 20, 20);

    // 1. Clear around the A button first (make background clean)
    int clear_radius = button_radius + 6;
    tft.fillCircle(circle_x, circle_y, clear_radius, TFT_BLACK);

    // 2. Draw shadow
    tft.fillCircle(circle_x + 4, circle_y + 4, button_radius, DARKER_SHADOW);

    // 3. Draw main red button
    tft.fillCircle(circle_x, circle_y, button_radius, TFT_RED);

    // 4. Highlight if selected
    if (waiting_select == 1) {
        tft.drawCircle(circle_x, circle_y, button_radius + 4, TFT_YELLOW);  // Yellow ring
    }

    // 5. Draw highlight spot
    tft.fillEllipse(circle_x - 7, circle_y - 7, 7, 5, LIGHTER_RED);

    // 6. Draw "A" letter
    tft.setTextColor(TFT_WHITE);
    tft.setTextSize(2);
    tft.setCursor(circle_x - 8, circle_y - 8);
    tft.print("A");
}
void draw_client_input_back(TFT_eSPI& tft, int client_select) {
    // --- Draw "Back" button ---
    tft.setTextSize(2);
    if (client_select == 0) {
        tft.setTextColor(TFT_BLACK, TFT_WHITE);  // Highlighted (white background)
    } else {
        tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    }
    tft.setCursor(10, 10);
    tft.print("< Back");
}


/////////////// Button Logic ////////////////////////

// Function to draw a button with shadow, highlight, and gray color
static void draw_button(TFT_eSPI& tft, Button* b, const String& state) {
    uint16_t buttonColor, textColor;
    uint16_t LIGHTER_GRAY = tft.color565(200, 200, 200);  // Lighter gray for highlight
    uint16_t DARKER_GRAY = tft.color565(100, 100, 100);   // Darker gray for shadow
    uint16_t GRAY = tft.color565(169, 169, 169);          // Custom gray color

    // Determine the button color based on state
    if (state == "hover") {
        buttonColor = LIGHTER_GRAY;  // Lighter color for hover effect
        textColor = TFT_BLACK;
    } else if (state == "pressed") {
        buttonColor = DARKER_GRAY;   // Darker color for pressed effect
        textColor = TFT_WHITE;
    } else {
        buttonColor = GRAY;  // Default gray color
        textColor = TFT_WHITE;
    }

    // Draw button with shadow
    // Shadow for button
    tft.fillRoundRect(b->x + 3, b->y + 3, b->w, b->h, 6, DARKER_GRAY);  // Shadow offset

    // Draw the button as a rounded rectangle
    tft.fillRoundRect(b->x, b->y, b->w, b->h, 6, buttonColor);
    
    // Set the text color
    tft.setTextColor(textColor);
    tft.setTextSize(2);

    // Draw the button label (the number or text like "Enter", "Del")
    tft.setCursor(b->x + (b->w - tft.textWidth(b->base)) / 2, b->y + (b->h - 16) / 2);
    tft.print(b->base);

    // Highlight effect
    if (state == "hover") {
        tft.fillEllipse(b->x + b->w - 10, b->y + 10, 10, 8, LIGHTER_GRAY);  // Highlight corner
    }
}

// Keep track of previously hovered button to avoid redundant redraws
int lastHoveredCol = -1;
int lastHoveredRow = -1;

// Draw a specific button in hover state only if it's different from the last hovered
void draw_button_hover(TFT_eSPI& tft, int col, int row) {
    if (col != lastHoveredCol || row != lastHoveredRow) {
        // Only redraw if the hovered button is different from the last hovered
        draw_button(tft, &pad[col][row], "hover");
        lastHoveredCol = col;
        lastHoveredRow = row;
    }
}

// Draw a specific button in pressed state
int draw_button_pressed(TFT_eSPI& tft, int col, int row) {  // Changed from LGFX to TFT_eSPI
    draw_button(tft, &pad[col][row], "pressed");
    return pad[col][row].button;
}

// Draw a specific button in the default state
void draw_button_default(TFT_eSPI& tft, int col, int row) {  // Changed from LGFX to TFT_eSPI
    draw_button(tft, &pad[col][row], "default");
}

// Initialize button properties with smaller button size and gaps
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
            b->x = BUTTON_START_X + col * (BUTTON_WIDTH + BUTTON_SPACING + 4);  // Slight gap
            b->y = BUTTON_START_Y + row * (BUTTON_HEIGHT + BUTTON_SPACING + 4);  // Slight gap
            b->w = BUTTON_WIDTH - 4;  // Slightly smaller width
            b->h = BUTTON_HEIGHT - 4;  // Slightly smaller height
            b->button = button_map[row][col];

            if (b->button == -1) b->base = "Enter";
            else if (b->button == -2) b->base = "Del";
            else b->base = String(b->button);
        }
    }
}

// Draw all buttons in default state
void draw_all_buttons(TFT_eSPI& tft) {  // Changed from LGFX to TFT_eSPI
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE, TFT_BLACK);  // Normal (black background)
    tft.setCursor(10, 10);
    tft.print("< Back");

    for (int row = 0; row < NUM_PAD_LENGTH; row++) {
        for (int col = 0; col < NUM_PAD_WIDTH; col++) {
            draw_button(tft, &pad[col][row], "default");
        }
    }
}

// Draw the code 
void draw_numbers(TFT_eSPI& tft, String code) {  // Changed from LGFX to TFT_eSPI
    String headerText = "Join a Host";

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
