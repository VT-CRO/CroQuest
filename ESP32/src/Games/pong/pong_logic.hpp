#pragma once

// Paddle dimensions
#define PADDLE_HEIGHT 50
#define PADDLE_WIDTH 10

// The number of points to win the game
#define GAME_WON 3

// Pong paddles
typedef struct {
  int x, y; /* (x, y) position of paddle */
  int w, h; /* width and height of paddle */
  bool paddle_mod;
} Paddle;

typedef struct {
  int x, y, w, h;
  bool paddle_mod;
} Prev_Paddle;

// Pong ball
typedef struct {
  double x, y;   /* (x, y) location of ball */
  int w, h;      /* width and height of ball */
  double dx, dy; /* x and y speed */
} Ball;

// Function declarations for C functions
void updatePaddle(bool up, Paddle *paddle);
void ai_paddle(Paddle *paddle, Ball *ball, int level_index);
void updateBall(Ball *ball, Paddle *paddles, int *level, int *score0,
                int *score1);
void initialize_game(Ball *ball, Paddle *paddles, int *level);
