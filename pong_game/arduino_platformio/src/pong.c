/** 
* Simple Pong 2-player 
* game using bluetooth
*/

#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <unistd.h>
#include <limits.h>
#include <math.h>

#include "utils.h"
#include "pong.h"

// PADDLE dimensions, speed and paddle deadzone
#define PADDLE_HEIGHT 60
#define PADDLE_WIDTH 10
#define PADDLE_SPEED 5
#define DEADZONE 10

// BALL dimensions and speed
#define BALL_HEIGHT 11
#define BALL_WIDTH 11
#define MAX_BALL_SPEED_Y 3.0
#define MAX_BALL_SPEED_X 15

#define NUM_PAD_LENGTH 4
#define NUM_PAD_WIDTH 3

// The three paddle walls used in
// collision detection
enum paddle_walls {
    TOP, 
    BOTTOM, 
    INSIDE,
};

/* Global Variables (static)  */

void initialize_game(Ball * ball, Paddle * paddles, int * level)
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

    *level = 1;

    ball->x = SCREEN_WIDTH / 2;
    ball->y = SCREEN_HEIGHT / 2;
    ball->w = BALL_WIDTH;
    ball->h = BALL_HEIGHT;
    ball->dx = dx;
    ball->dy = dy;

    paddles[0] = (Paddle){0, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
    paddles[1] = (Paddle){SCREEN_WIDTH - PADDLE_WIDTH, (SCREEN_HEIGHT / 2) - (PADDLE_HEIGHT / 2), PADDLE_WIDTH, PADDLE_HEIGHT};
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

/*
Checks if the ball has collided with either
paddle
*/
static void check_paddle_collision(Ball * ball, Paddle * paddles, int * level)
{
    // TODO - speedup ball when hitting paddle and change ball dx and dy based on 
    // where the ball hits the paddle

    // Check collision with left paddle
    if(!(ball->x + ball->w < paddles[0].x ||
         ball->x > paddles[0].x + PADDLE_WIDTH ||
         ball->y + ball->h < paddles[0].y ||
         ball->y > paddles[0].y + PADDLE_HEIGHT)) {

        int inside_dist = abs((ball->x) - (paddles[0].x + PADDLE_WIDTH));
        int top_dist = abs((ball->y + ball->h) - paddles[0].y);
        int bottom_dist = abs(ball->y - (paddles[0].y + PADDLE_HEIGHT));

        double paddle_center = paddles[0].y + PADDLE_HEIGHT / 2;
        int offset = ball->y - paddle_center;
        float normalized_offset = (float)offset / (PADDLE_HEIGHT / 2);
        ball->dy = normalized_offset * MAX_BALL_SPEED_Y;
        
        if (fabs(ball->dy) < 1.0f) {
            ball->dy = (ball->dy < 0 ? -1.0f : 1.0f);
        }

        enum paddle_walls minimum = closest_wall(paddles[0], inside_dist, top_dist, bottom_dist);
        if(minimum == TOP) {
            ball->y = paddles[0].y - ball->h;
            ball->dy = -ball->dy;
        } else if(minimum == BOTTOM) {
            ball->y = paddles[0].y + PADDLE_HEIGHT;
            ball->dy = -ball->dy;
        } else {
            ball->x = paddles[0].x + PADDLE_WIDTH;
            ball->dx = -ball->dx * 1.2;
        }
    }
    // Check collision with right paddle
    else if(!(ball->x + ball->w < paddles[1].x ||
              ball->x > paddles[1].x + PADDLE_WIDTH ||
              ball->y + ball->h < paddles[1].y ||
              ball->y > paddles[1].y + PADDLE_HEIGHT)) {

        int inside_dist = abs(ball->x + ball->w - paddles[1].x);
        int top_dist = abs((ball->y + ball->h) - paddles[1].y);
        int bottom_dist = abs(ball->y - (paddles[1].y + PADDLE_HEIGHT));

        double paddle_center = paddles[1].y + PADDLE_HEIGHT / 2;
        int offset = ball->y - paddle_center;
        float normalized_offset = (float)offset / (PADDLE_HEIGHT / 2);
        ball->dy = normalized_offset * MAX_BALL_SPEED_Y;

        if (fabs(ball->dy) < 1.0f) {
            ball->dy = (ball->dy < 0 ? -1.0f : 1.0f);
        }

        enum paddle_walls minimum = closest_wall(paddles[1], inside_dist, top_dist, bottom_dist);
        if(minimum == TOP) {
            ball->y = paddles[1].y - ball->h;
            ball->dy = -ball->dy;
        } else if(minimum == BOTTOM) {
            ball->y = paddles[1].y + PADDLE_HEIGHT;
            ball->dy = -ball->dy;
        } else {
            ball->x = paddles[1].x - ball->w;
            ball->dx = -ball->dx * 1.2;
        }
        (*level)++; 
    }

    if (fabs(ball->dx) > MAX_BALL_SPEED_X) {
        ball->dx = (ball->dx > 0 ? MAX_BALL_SPEED_X : -MAX_BALL_SPEED_X);
    }

}

/*
Deals with ball collision
*/
static void ball_collision(Ball * ball, Paddle * paddles, int * level, int * score0, int * score1)
{
    
    //Check if a point was scored
    if(ball->x <= 0 || ball->x + ball->w >= SCREEN_WIDTH){
        if(ball->x <= 0){
            (*score1)++;
        }else{
            (*score0)++;
        }

        initialize_game(ball, paddles, level);
        return;
    }
    
    //Hits top or bottom of screen
    if(ball->y <= 0){
        ball->y = 0;
        ball->dy = -ball->dy;
    }else if(ball->y + ball->h >= SCREEN_HEIGHT){
        ball->y = SCREEN_HEIGHT - ball->h;
        ball->dy = -ball->dy;
    }
    
    //Check paddle collision
    check_paddle_collision(ball, paddles, level);
}

/*
Updates the ball (position and speed)

*/
void updateBall(Ball * ball, Paddle * paddles, int * level, int * score0, int * score1)
{
    ball_collision(ball, paddles, level, score0, score1);
    ball->x += ball->dx;
    ball->y += ball->dy;
}

/*
Updates the paddle position
*/
void updatePaddle(bool up, Paddle * paddle)
{
    // Moves the paddle up
    if(up){
        paddle->y = max(paddle->y - PADDLE_SPEED, 0);
    }
    //Moves the paddle down
    else{
        paddle->y = min(paddle->y + PADDLE_SPEED, SCREEN_HEIGHT - PADDLE_HEIGHT);
    }
}


/*
AI paddle for single player
*/
void ai_paddle(Paddle * paddle, Ball * ball, int level_index)
{
    if(ball->dx > 0)
        return; 
    double intersection_time = ((paddle->x + PADDLE_WIDTH) - ball->x) / ball->dx;
    double y_intersect = ball->y + (ball->dy * intersection_time);

    y_intersect = y_intersect >= SCREEN_HEIGHT ? SCREEN_HEIGHT - PADDLE_HEIGHT : y_intersect;
    y_intersect = y_intersect < 0 ? 0 : y_intersect;

    double target_y = ball->dy > 0 ? y_intersect - calculate_offset(level_index, PADDLE_HEIGHT) : y_intersect + calculate_offset(level_index, PADDLE_HEIGHT);
    target_y = target_y >= SCREEN_HEIGHT ? SCREEN_HEIGHT - PADDLE_HEIGHT : target_y;
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