#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// Screen dimensions subject to change
// 250 200
#define SCREEN_HEIGHT 240
#define SCREEN_WIDTH 320

// The number of points to win the game
#define GAME_WON 3

typedef enum {
    STATE_HOME,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_MULTIPLAYER,
    STATE_BLUETOOTH_CLIENT,
    STATE_BLUETOOTH_START,
} GameState;

// Pong paddles
typedef struct {
    int x, y;  /* (x, y) position of paddle */
    int w, h;  /* width and height of paddle */
} Paddle;

// Pong ball
typedef struct {
    double x, y;   /* (x, y) location of ball */
    int w, h;      /* width and height of ball */
    double dx, dy; /* x and y speed */
} Ball;

// Function declarations for C functions
void updatePaddle(bool up, Paddle* paddle);
void ai_paddle(Paddle* paddle, Ball* ball, int level_index);
void updateBall(Ball* ball, Paddle* paddles, int* level, int* score0, int* score1);
void initialize_game(Ball* ball, Paddle* paddles, int* level);

#ifdef __cplusplus
}
#endif
