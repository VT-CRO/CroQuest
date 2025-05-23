/** 
* Simple Pong 2-player 
* game using bluetooth
*/

#include "pong_logic.hpp"
#include <Arduino.h>

#define SPEAKER_PIN 21 

//Screen dimensions
const int SCREEN_HEIGHT = 320;
const int SCREEN_WIDTH = 480;

// PADDLE speed and paddle deadzone
#define PADDLE_SPEED 5
#define DEADZONE 10

// BALL dimensions and speed
#define BALL_HEIGHT 11
#define BALL_WIDTH 11
#define MAX_BALL_SPEED_Y 3.0
#define MAX_BALL_SPEED_X 10

#define NUM_PAD_LENGTH 4
#define NUM_PAD_WIDTH 3

// The three paddle walls used in
// collision detection
enum paddle_walls {
    TOP, 
    BOTTOM, 
    INSIDE,
};

// ============ SOUNDS ============= //

void playPaddleHitSound() {
  const int channel = 0;
  const int freq = 900;     // Sharp "blip"
  const int duration = 20;  // Very quick

  ledcAttachPin(SPEAKER_PIN, channel);
  ledcWriteTone(channel, freq);
  delay(duration);
  ledcWriteTone(channel, 0);
}

void playWallBounceSound() {
  const int channel = 0;
  const int freq = 700;     // Slightly lower pitch
  const int duration = 20;

  ledcAttachPin(SPEAKER_PIN, channel);
  ledcWriteTone(channel, freq);
  delay(duration);
  ledcWriteTone(channel, 0);
}

void playMissSound() {
  const int channel = 0;
  ledcAttachPin(SPEAKER_PIN, channel);

  // Rising tone: 400 Hz → 700 Hz
  for (int freq = 400; freq <= 700; freq += 30) {
    ledcWriteTone(channel, freq);
    delay(15); // Smooth upward sweep
  }

  ledcWriteTone(channel, 0); // Turn off sound
}


/*
Returns the minimum value
*/
int min(int value1, int value2)
{
    return value1 >= value2 ? value2 : value1;
}

/*
Returns the maximum value
*/
int max(int value1, int value2)
{
    return value1 >= value2 ? value1 : value2;
}

/* Global Variables (static)  */

void initialize_game(Ball * ball, Paddle * paddles, int * level)
{
    double dx = -4;
    double dy = -4;
    // Simple direction picker
    if(random() % 2 == 0)
    {
        // Right direction
        dx = 4;
        if(random() % 2 == 0){
            dy = 4;
        }else{
            dy = -4;
        }
    }else{
        // Left direction
        dx = -4;
        if(random() % 2 == 0)
        {
            dy = 4;
        }else{
            dx = -4;
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
static void check_paddle_collision(Ball *ball, Paddle *paddles, int *level)
{
    for (int i = 0; i < 2; i++) {
        Paddle *paddle = &paddles[i];
        
        // Check if ball is colliding with this paddle
        if (ball->x + ball->w < paddle->x ||
            ball->x > paddle->x + PADDLE_WIDTH ||
            ball->y + ball->h < paddle->y ||
            ball->y > paddle->y + PADDLE_HEIGHT) {
            continue;  // No collision with this paddle
        }
        
        // Calculate distances to each wall
        int inside_dist = (i == 0) ? 
            abs(ball->x - (paddle->x + PADDLE_WIDTH)) : 
            abs(ball->x + ball->w - paddle->x);
        int top_dist = abs((ball->y + ball->h) - paddle->y);
        int bottom_dist = abs(ball->y - (paddle->y + PADDLE_HEIGHT));
        
        // Find which wall was hit
        enum paddle_walls minimum = closest_wall(*paddle, inside_dist, top_dist, bottom_dist);
        
        // Calculate vertical bounce angle based on where ball hits paddle
        double paddle_center = paddle->y + PADDLE_HEIGHT / 2;
        float normalized_offset = (float)(ball->y - paddle_center) / (PADDLE_HEIGHT / 2);
        
        // Apply a sinusoidal curve to move max velocity points inward
        // This creates max velocity at around 2/3 from center instead of at the edge
        normalized_offset = sinf(normalized_offset * 1.2f * M_PI / 2.0f);
        
        // Add randomization for additional variation (±20%)
        float random_factor = 1.0f + ((random() % 40) - 20) / 100.0f;
        normalized_offset *= random_factor;
        
        // Occasional "wild bounce" for extreme variation (8% chance)
        if (random() % 100 < 8) {
            // Create more dramatic angles occasionally 
            float wild_factor = 1.3f + (random() % 40) / 100.0f; // 1.3 to 1.7
            normalized_offset *= wild_factor;
            // Sometimes reverse expected direction for surprising effect
            if (random() % 100 < 30) { // 30% of wild bounces (2.4% overall)
                normalized_offset = -normalized_offset;
            }
        }
        
        // Set the Y velocity with our enhanced variation
        ball->dy = normalized_offset * MAX_BALL_SPEED_Y;
        
        // Ensure minimum vertical movement
        float min_speed = 2.0f;
        if (fabs(ball->dy) < min_speed) {
            ball->dy = (ball->dy < 0 ? -min_speed : min_speed);
        }
        
        // Handle collision based on wall hit
        if (minimum == TOP) {
            ball->y = paddle->y - ball->h;
            ball->dy = -fabs(ball->dy);  // Always bounce up
        } else if (minimum == BOTTOM) {
            ball->y = paddle->y + PADDLE_HEIGHT;
            ball->dy = fabs(ball->dy);   // Always bounce down
        } else {
            // Side collision - set position and reverse horizontal direction
            if (i == 0) {  // Left paddle
                ball->x = paddle->x + PADDLE_WIDTH;
                ball->dx = fabs(ball->dx) * (1.0 + (*level) * 0.05);  // Gradual speed increase
            } else {       // Right paddle
                ball->x = paddle->x - ball->w;
                ball->dx = -fabs(ball->dx) * (1.0 + (*level) * 0.05);
                (*level)++;  // Increase level on right paddle hit
            }
        }
        
        // Cap horizontal speed
        if (fabs(ball->dx) > MAX_BALL_SPEED_X) {
            ball->dx = (ball->dx > 0 ? MAX_BALL_SPEED_X : -MAX_BALL_SPEED_X);
        }
        
        // Also cap vertical speed (with a higher limit for extra variation)
        float max_y_speed = MAX_BALL_SPEED_Y * 1.5f;
        if (fabs(ball->dy) > max_y_speed) {
            ball->dy = (ball->dy > 0 ? max_y_speed : -max_y_speed);
        }

        playPaddleHitSound();
        
        // We've handled a collision, no need to check the other paddle
        return;
    }
}

/*
Deals with ball collision
*/
static void ball_collision(Ball * ball, Paddle * paddles, int * level, int * score0, int * score1)
{
    
    //Check if a point was scored
    if(ball->x <= 0 || ball->x + ball->w >= SCREEN_WIDTH){
        playMissSound();
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
        playWallBounceSound();
        ball->y = 0;
        ball->dy = -ball->dy;
    }else if(ball->y + ball->h >= SCREEN_HEIGHT){
        playWallBounceSound();
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
void ai_paddle(Paddle *paddle, Ball *ball, int level_index)
{
    // Only react if the ball is moving toward the AI
    // if (ball->dx > 0)
        // return;

    if (ball->x > SCREEN_WIDTH / 2) {
        double paddle_center = paddle->y + PADDLE_HEIGHT / 2;
    
        // Curiosity strength increases as the ball approaches halfway
        double curiosity_factor = 1.0 - ((ball->x - SCREEN_WIDTH / 2) / (SCREEN_WIDTH / 2));
        if (curiosity_factor < 0) curiosity_factor = 0;
        if (curiosity_factor > 1) curiosity_factor = 1;
    
        // 30% chance to move when curiosity is high, 5% when it's low
        int chance = 5 + (int)(25 * curiosity_factor);
        if ((random() % 100) < chance) {
            // Target near ball with some randomness
            double target_y = ball->y + ((random() % 21) - 10); // ±10px noise
    
            if (target_y < paddle_center - DEADZONE) {
                updatePaddle(true, paddle); // move up
            } else if (target_y > paddle_center + DEADZONE) {
                updatePaddle(false, paddle); // move down
            }
        }
        return;
    }

    double intersection_time = ((paddle->x + PADDLE_WIDTH) - ball->x) / ball->dx;
    double y_intersect = ball->y + (ball->dy * intersection_time);

    // Clamp y intersection within screen bounds
    y_intersect = y_intersect >= SCREEN_HEIGHT ? SCREEN_HEIGHT - PADDLE_HEIGHT : y_intersect;
    y_intersect = y_intersect < 0 ? 0 : y_intersect;

    // Ball speed and difficulty-adjusted error
    double ball_speed = sqrt(ball->dx * ball->dx + ball->dy * ball->dy);
    double mistake_range = (ball_speed / 5.0) * (1.0 / (intersection_time + 0.1)); // avoid div by 0

    // Scale error with level (higher level = smaller mistake)
    double difficulty_factor = 1.0 + (level_index * 0.1);
    mistake_range *= 1.0 / difficulty_factor;

    // Clamp mistake range
    if (mistake_range > PADDLE_HEIGHT / 2) mistake_range = PADDLE_HEIGHT / 2;
    if (mistake_range < 2) mistake_range = 2;

    // Add error to the target
    double error = ((random() % 100) / 100.0 - 0.5) * 2 * mistake_range; // [-range, +range]
    double target_y = y_intersect + error;

    // Clamp within screen bounds
    target_y = target_y >= SCREEN_HEIGHT ? SCREEN_HEIGHT - PADDLE_HEIGHT : target_y;
    target_y = target_y < 0 ? 0 : target_y;

    double paddle_center = paddle->y + PADDLE_HEIGHT / 2;

    if (target_y < paddle_center - DEADZONE &&
        intersection_time > 0 &&
        intersection_time < fabs(target_y - paddle_center) / PADDLE_SPEED)
    {
        updatePaddle(true, paddle);
    }
    if (target_y > paddle_center + DEADZONE &&
        intersection_time > 0 &&
        intersection_time < fabs(target_y - paddle_center) / PADDLE_SPEED)
    {
        updatePaddle(false, paddle);
    }
}
