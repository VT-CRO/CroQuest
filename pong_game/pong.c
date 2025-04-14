/** 
* Simple Pong 2-player 
* game using bluetooth
*/

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_main.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>

#include "utils.h"
#include "pong.h"
#include "draw.h"

//PADDLE dimensions
// 50, 10
#define PADDLE_HEIGHT 60
#define PADDLE_WIDTH 10

//BALL dimensions
#define BALL_HEIGHT 11
#define Ball_WIDTH 11
#define MAX_BALL_SPEED_Y 3.0
#define MAX_BALL_SPEED_X 15

#define PADDLE_SPEED 5
#define DEADZONE 10

#define GAME_WON 3

// Subject to change - Pins to
// read up and down button presses
#define UP SDL_SCANCODE_UP
#define DOWN SDL_SCANCODE_DOWN

// The three paddle walls used in
// collision detection
enum paddle_walls {
    TOP, 
    BOTTOM, 
    INSIDE,
};

//The game difficulty level
static int level;

// State
static GameState current_state = STATE_HOME;

// Player scores
static int score[2] = {0, 0};

//Ball and Paddles
static Ball ball;
static Paddle paddles[2];

static void initialize_game()
{
    double dx = -1;
    double dy = -1;
    // Simple direction picker
    if(rand() % 2 == 0)
    {
        // Right direction
        dx = 1;
        if(rand() % 2 == 0){
            dy = 1;
        }else{
            dy = -1;
        }
    }else{
        // Left direction
        dx = -1;
        if(rand() % 2 == 0)
        {
            dy = 1;
        }else{
            dx = -1;
        }
    }

    level = 1;

    ball = (Ball) {SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, BALL_HEIGHT, 
        Ball_WIDTH, dx, dy};
    
    paddles[0] = (Paddle){0, (SCREEN_WIDTH / 2) - (PADDLE_HEIGHT / 2),
        PADDLE_WIDTH, PADDLE_HEIGHT};
    paddles[1] = (Paddle){SCREEN_HEIGHT - PADDLE_WIDTH,
         (SCREEN_WIDTH / 2) - (PADDLE_HEIGHT / 2), 
         PADDLE_WIDTH, PADDLE_HEIGHT};
}


/////// GAME LOGIC ////////////
/* 
Checks which paddle wall the ball is closest to 
*/
static enum paddle_walls closest_wall(Paddle paddle, int inside_dist, int top_dist, int bottom_dist){
    enum paddle_walls minimum = TOP;
    int minimum_value = top_dist;

    if(bottom_dist < minimum_value){
        minimum = BOTTOM;
        minimum_value = bottom_dist;
    }
    if(inside_dist < minimum_value){
        minimum = INSIDE;
        minimum_value = inside_dist;
    }
    
    return minimum;
}

float getRandomVariance(float min, float max) {
    // Ensure random is seeded (do this once in program initialization)
    static int seeded = 0;
    if (!seeded) {
        srand(time(NULL));
        seeded = 1;
    }
    
    float range = max - min;
    return min + ((float)rand() / RAND_MAX) * range;
}
/*
Checks if the ball has collided with either
paddle
*/
static void check_paddle_collision()
{
    // TODO - speedup ball when hitting paddle and change ball dx and dy based on 
    // where the ball hits the paddle

    // Check collision with left paddle
    if(!(ball.x + ball.w < paddles[0].x ||
         ball.x > paddles[0].x + PADDLE_WIDTH ||
         ball.y + ball.h < paddles[0].y ||
         ball.y > paddles[0].y + PADDLE_HEIGHT)) {

        int inside_dist = abs((ball.x) - (paddles[0].x + PADDLE_WIDTH));
        int top_dist = abs((ball.y + ball.h) - paddles[0].y);
        int bottom_dist = abs(ball.y - (paddles[0].y + PADDLE_HEIGHT));

        double paddle_center = paddles[0].y + PADDLE_HEIGHT / 2;
        int offset = ball.y - paddle_center;
        float normalized_offset = (float)offset / (PADDLE_HEIGHT / 2);
        ball.dy = normalized_offset * MAX_BALL_SPEED_Y;
        
        if (fabs(ball.dy) < 1.0f) {
            ball.dy = (ball.dy < 0 ? -1.0f : 1.0f);
        }

        enum paddle_walls minimum = closest_wall(paddles[0], inside_dist, top_dist, bottom_dist);
        if(minimum == TOP) {
            ball.y = paddles[0].y - ball.h;
            ball.dy = -ball.dy;
        } else if(minimum == BOTTOM) {
            ball.y = paddles[0].y + PADDLE_HEIGHT;
            ball.dy = -ball.dy;
        } else {
            ball.x = paddles[0].x + PADDLE_WIDTH;
            ball.dx = -ball.dx * 1.2;
        }
    }
    // Check collision with right paddle
    else if(!(ball.x + ball.w < paddles[1].x ||
              ball.x > paddles[1].x + PADDLE_WIDTH ||
              ball.y + ball.h < paddles[1].y ||
              ball.y > paddles[1].y + PADDLE_HEIGHT)) {

        int inside_dist = abs(ball.x + ball.w - paddles[1].x);
        int top_dist = abs((ball.y + ball.h) - paddles[1].y);
        int bottom_dist = abs(ball.y - (paddles[1].y + PADDLE_HEIGHT));

        double paddle_center = paddles[1].y + PADDLE_HEIGHT / 2;
        int offset = ball.y - paddle_center;
        float normalized_offset = (float)offset / (PADDLE_HEIGHT / 2);
        ball.dy = normalized_offset * MAX_BALL_SPEED_Y;

        if (fabs(ball.dy) < 1.0f) {
            ball.dy = (ball.dy < 0 ? -1.0f : 1.0f);
        }

        enum paddle_walls minimum = closest_wall(paddles[1], inside_dist, top_dist, bottom_dist);
        if(minimum == TOP) {
            ball.y = paddles[1].y - ball.h;
            ball.dy = -ball.dy;
        } else if(minimum == BOTTOM) {
            ball.y = paddles[1].y + PADDLE_HEIGHT;
            ball.dy = -ball.dy;
        } else {
            ball.x = paddles[1].x - ball.w;
            ball.dx = -ball.dx * 1.2;
        }
        level++;
    }

    if (fabs(ball.dx) > MAX_BALL_SPEED_X) {
        ball.dx = (ball.dx > 0 ? MAX_BALL_SPEED_X : -MAX_BALL_SPEED_X);
    }

}

/*
Deals with ball collision
*/
static void ball_collision()
{
    
    //Check if a point was scored
    if(ball.x <= 0 || ball.x + ball.w >= SCREEN_HEIGHT){
        if(ball.x <= 0){
            score[0]++;
        }else{
            score[1]++;
        }

        if(score[0] >= GAME_WON || score[1] >= GAME_WON)
        {
            current_state = STATE_GAMEOVER;
        }

        initialize_game();
        return;
    }
    
    //Hits top or bottom of screen
    if(ball.y <= 0){
        ball.y = 0;
        ball.dy = -ball.dy;
    }else if(ball.y + ball.h >= SCREEN_WIDTH){
        ball.y = SCREEN_WIDTH - ball.h;
        ball.dy = -ball.dy;
    }
    
    //Check paddle collision
    check_paddle_collision();
}

/*
Updates the ball (position and speed)

*/
static void updateBall()
{
    ball_collision();
    ball.x += ball.dx;
    ball.y += ball.dy;
}

/*
Updates the paddle position
*/
static void updatePaddle(bool up, Paddle * paddle)
{
    // Moves the paddle up
    if(up){
        paddle->y = max(paddle->y - PADDLE_SPEED, 0);
    }
    //Moves the paddle down
    else{
        paddle->y = min(paddle->y + PADDLE_SPEED, SCREEN_WIDTH - PADDLE_HEIGHT);
    }
}


/*
AI paddle for single player
*/
static void ai_paddle(Paddle * paddle, int level_index)
{
    if(ball.dx > 0)
        return; 
    double intersection_time = ((paddle->x + PADDLE_WIDTH) - ball.x) / ball.dx;
    double y_intersect = ball.y + (ball.dy * intersection_time);

    y_intersect = y_intersect >= SCREEN_WIDTH ? SCREEN_WIDTH - PADDLE_HEIGHT : y_intersect;
    y_intersect = y_intersect < 0 ? 0 : y_intersect;

    double target_y = ball.dy > 0 ? y_intersect - calculate_offset(level_index, PADDLE_HEIGHT) : y_intersect + calculate_offset(level_index, PADDLE_HEIGHT);
    target_y = target_y >= SCREEN_WIDTH ? SCREEN_WIDTH - PADDLE_HEIGHT : target_y;
    target_y = target_y < 0 ? 0 : target_y;
    

    double paddle_center = paddle->y + PADDLE_HEIGHT / 2;

    if(target_y < paddle_center - DEADZONE && intersection_time > 0 && 
        intersection_time < abs(target_y - paddle_center) / PADDLE_SPEED)
     {
         updatePaddle(true, paddle);
     }
     
     if(target_y > paddle_center + DEADZONE && intersection_time > 0 &&
        intersection_time < abs(target_y - paddle_center) / PADDLE_SPEED)
     {
         updatePaddle(false, paddle);
     }
}

/*
Main logic/game loop
*/
int main()
{
    //initializes srand using time
    srand((unsigned int)time(NULL));

    init_drawing();

    //main loop
    bool running = true;
    SDL_Event event;
    static bool game_initialized = false;
    while(running)
    {
        while (SDL_PollEvent(&event)) {
            //// Exit Logic - 3 different ways to exit ////

            // Exit button
            if (event.type == SDL_QUIT) {
                running = false;
            }
            else if(event.type == SDL_KEYDOWN)
            {
                // Press q to exit
                if(event.key.keysym.sym == SDLK_q){
                    running = false;
                }
                // Press esc to exit
                else if(event.key.keysym.sym == SDLK_ESCAPE){
                    running = false;
                }
                else if(event.key.keysym.sym == SDLK_b){
                    if(current_state == STATE_HOME || current_state == STATE_BLUETOOTH_HOST){
                        current_state = STATE_PLAYING;
                    }
                }
                else if(event.key.keysym.sym == SDLK_a){
                    if(current_state == STATE_HOME){
                        current_state = STATE_BLUETOOTH_HOST;
                    }
                }
                else if(event.key.keysym.sym == SDLK_SPACE){
                    if(current_state == STATE_GAMEOVER){
                        current_state = STATE_PLAYING;
                        score[0] = 0;
                        score[1] = 0;
                    }
                }
            }
        }

        if (current_state == STATE_HOME) {
            draw_homescreen();
        }
        else if(current_state == STATE_BLUETOOTH_HOST){
            draw_hostcode();
        }
        else if(current_state == STATE_PLAYING)
        {
            if (!game_initialized) {
                initialize_game();
                game_initialized = true;
            }
            // current keyboard state
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            
            //Up arrow key pressed
            if (keystate[UP]) {
                updatePaddle(true, &paddles[1]);
            }
    
            //Down arrow key pressed
            if (keystate[DOWN]) {
                updatePaddle(false, &paddles[1]);
            }
            
            //Updates everything
            
            ai_paddle(&paddles[0], level);
            updateBall();
            draw_playing(paddles, ball, score);
        }
        else if(current_state == STATE_GAMEOVER){
            draw_endgame();
        }


    }
    destroy();
}