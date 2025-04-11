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

// Screen dimensions subject to change
// 250 200
#define SCREEN_HEIGHT 250
#define SCREEN_WIDTH 200

//PADDLE dimensions
// 50, 10
#define PADDLE_HEIGHT 50
#define PADDLE_WIDTH 10

//BALL dimensions
#define BALL_HEIGHT 5
#define Ball_WIDTH 5
#define MAX_BALL_SPEED_Y 3.0

#define PADDLE_SPEED 5
#define DEADZONE 10

// Subject to change - Pins to
// read up and down button presses
#define UP SDL_SCANCODE_UP
#define DOWN SDL_SCANCODE_DOWN

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

enum paddle_walls {
    TOP, 
    BOTTOM, 
    INSIDE,
};

typedef enum {
    STATE_HOME,
    STATE_PLAYING,
} GameState;

//The game difficulty level
static int level;

//Ball and Paddles initialized


static Ball ball;

static Paddle paddles[2];

static void initialize_game()
{
    double dx;
    double dy;
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

    ball = (Ball) {SCREEN_HEIGHT / 2, SCREEN_WIDTH / 2, BALL_HEIGHT, 
        Ball_WIDTH, dx, dy};
    
    paddles[0] = (Paddle){0, (SCREEN_WIDTH / 2) - (PADDLE_HEIGHT / 2),
        PADDLE_WIDTH, PADDLE_HEIGHT};
    paddles[1] = (Paddle){SCREEN_HEIGHT - PADDLE_WIDTH,
         (SCREEN_WIDTH / 2) - (PADDLE_HEIGHT / 2), 
         PADDLE_WIDTH, PADDLE_HEIGHT};
}

////// DRAWING ASSETS /////////

/*
Draw the paddles 

*/
static void draw_paddles(SDL_Renderer * renderer)
{
    for(int i = 0; i < 2; i++)
    {
        SDL_Rect rect = {paddles[i].x, paddles[i].y, paddles[i].w, paddles[i].h};
        if(SDL_RenderFillRect(renderer, &rect) < 0){
            fprintf(stderr, "Ball drawing failed");
        }
    }
}

/*
Draw the ball 

*/
static void draw_ball(SDL_Renderer * renderer)
{
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect rect = {ball.x, ball.y, ball.w, ball.h};
    if(SDL_RenderFillRect(renderer, &rect) < 0){
        fprintf(stderr, "Ball drawing failed");
    }
}

/*
Draw homescreen
*/
void draw_home_screen(SDL_Renderer* renderer, SDL_Texture* homeScreenTexture) {
    SDL_RenderClear(renderer);
    SDL_Rect dst = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
    SDL_RenderCopy(renderer, homeScreenTexture, NULL, &dst);
    SDL_RenderPresent(renderer);
}

/*
Draw the 2 players score

*/
static void draw_scores()
{

}       

/*
Draw the net/line separating both sides

*/
static void draw_net()
{

}

/*
Draw the board

*/
static void draw_board()
{

}

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

/////// GAME LOGIC ////////////

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
}

/*
Deals with ball collision
*/
static void ball_collision()
{
    //Check if a point was scored
    if(ball.x <= 0 || ball.x + ball.w >= SCREEN_HEIGHT){
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

    double target_y = y_intersect + calculate_offset(level_index, PADDLE_HEIGHT);

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

    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        fprintf(stderr, "Failed to initialize SDL\n");
        exit(1);
    }

    SDL_Window* window;
    if((window = SDL_CreateWindow("PONG",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_HEIGHT, SCREEN_WIDTH,
        SDL_WINDOW_SHOWN)) == NULL){
            fprintf(stderr,"Failed to create window\n");
            SDL_Quit();
            exit(1);
        }
    
    //Initializes renderer
    SDL_Renderer* renderer;
    if((renderer = SDL_CreateRenderer(window, -1, 0)) == NULL){
        fprintf(stderr, "Failed to create renderer\n");
        SDL_Quit();
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
    }

    SDL_Texture* backgroundTexture = NULL;
    SDL_Surface* tempSurface = IMG_Load("./assets/background.png");
    if (!tempSurface) {
        fprintf(stderr, "Failed to load background: %s\n", IMG_GetError());
        SDL_Quit();
        return 1;
    }

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (!backgroundTexture) {
        fprintf(stderr, "Failed to create background texture: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }
    SDL_Texture* homeScreenTexture = NULL;
    SDL_Surface* homeSurface = IMG_Load("./assets/home_screen.png");
    if (!homeSurface) {
        fprintf(stderr, "Failed to load home screen image: %s\n", IMG_GetError());
        exit(1);
    }
    homeScreenTexture = SDL_CreateTextureFromSurface(renderer, homeSurface);
    SDL_FreeSurface(homeSurface);
    if (!homeScreenTexture) {
        fprintf(stderr, "Failed to create home screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    //main loop
    bool running = true;
    SDL_Event event;
    GameState current_state = STATE_HOME;
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
                    current_state = STATE_PLAYING;
                }
            }
        }

        if (current_state == STATE_HOME) {
            draw_home_screen(renderer, homeScreenTexture);
        }
        else if(current_state == STATE_PLAYING)
        {
            // current keyboard state
            const Uint8 *keystate = SDL_GetKeyboardState(NULL);
            
            //Up arrow key pressed
            if (keystate[UP]) {
                updatePaddle(true, &paddles[1]);
                // updateBall();
            }
    
            //Down arrow key pressed
            if (keystate[DOWN]) {
                updatePaddle(false, &paddles[1]);
                // updateBall();
            }
            if(keystate[SDL_SCANCODE_RIGHT]){
                paddles[1].x--;
            }
            if(keystate[SDL_SCANCODE_LEFT]){
                paddles[1].x++;
            }
    
            //Updates everything
    
            ai_paddle(&paddles[0], level);
            updateBall();
            draw_ball(renderer);
            draw_paddles(renderer);
            SDL_RenderPresent(renderer);
    
            SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            SDL_RenderClear(renderer);
            SDL_Rect dst = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
            SDL_RenderCopy(renderer, backgroundTexture, NULL, &dst);
    
            //hard codes a delay
            SDL_Delay(10);
        }

    }
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(homeScreenTexture);
    SDL_DestroyWindow(window);
    SDL_Quit();
}