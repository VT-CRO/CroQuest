// Screen dimensions subject to change
// 250 200
#define SCREEN_HEIGHT 460
#define SCREEN_WIDTH 300

typedef enum {
    STATE_HOME,
    STATE_PLAYING,
    STATE_GAMEOVER,
    STATE_BLUETOOTH_HOST,
} GameState;

//Pong paddles
typedef struct
{
    int x, y;  /* (x, y) position of paddle  */
    int w, h;  /* width and height of ball */
}Paddle;

//Pong ball
typedef struct
{
    double x, y;   /* (x, y) location of ball */
    int w, h;   /* width and height of ball */
    double dx, dy; /* x and y speed */
}Ball;