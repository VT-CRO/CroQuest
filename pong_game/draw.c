#include <stdbool.h>
#include "pong.h"
#include "draw.h"

#define NUM_WIDTH 80

static SDL_Window * window;
static SDL_Renderer* renderer;
static SDL_Texture* backgroundTexture;
static SDL_Texture * hostcodeTexture;
static SDL_Texture* homeScreenTexture;
static SDL_Texture* ballTexture;
static SDL_Surface* numbers;
static SDL_Texture* numTexture;


/////////////////////// HELPER FUNCTIONS ////////////////////////////

/*
Draw the 2 players score

*/
static void draw_scores(SDL_Renderer* renderer, SDL_Texture* numTexture, int * score)
{
    int pos[2] = {((SCREEN_HEIGHT) / 2) + (NUM_WIDTH / 3), ((SCREEN_HEIGHT) / 2) - (2*(NUM_WIDTH / 3))};
    SDL_Rect sourceRect = {0, 0, NUM_WIDTH, numbers->h};
    SDL_Rect destRect = {0, 0, sourceRect.w / 3, sourceRect.h / 3};
    
    for(int i = 0; i < 2; i++){
        if(score[i] / 10 == 0){
            sourceRect.x = score[i] * NUM_WIDTH;
            destRect.x = pos[i];
            SDL_RenderCopy(renderer, numTexture, &sourceRect, &destRect);
        }
        else{
            sourceRect.x = (score[i] / 10) * NUM_WIDTH;
            destRect.x = i == 1  ? pos[i] - sourceRect.w / 3 : pos[i];
            SDL_RenderCopy(renderer, numTexture, &sourceRect, &destRect);

            sourceRect.x = (score[i] % 10) * NUM_WIDTH;
            destRect.x = i == 1 ? pos[i] : pos[i] + (sourceRect.w / 3);
            SDL_RenderCopy(renderer, numTexture, &sourceRect, &destRect);
        }
    }
}

/*
Draw the paddles 

*/
static void draw_paddles(SDL_Renderer * renderer, Paddle * paddles)
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
static void draw_ball(SDL_Renderer * renderer, SDL_Texture* ballTexture, Ball ball)
{
    SDL_Rect rect = {ball.x, ball.y, ball.w, ball.h};
    SDL_RenderCopy(renderer, ballTexture, NULL, &rect);
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
}

/*
Draw homescreen
*/
static void draw_home_screen(SDL_Renderer* renderer, SDL_Texture* homeScreenTexture) {
    SDL_RenderClear(renderer);
    SDL_Rect dst = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
    SDL_RenderCopy(renderer, homeScreenTexture, NULL, &dst);
    SDL_RenderPresent(renderer);
}
/*
Draw hostcode screen
*/
static void hostcode_screen(SDL_Renderer* renderer, SDL_Texture* hostcodeTexture){
    SDL_RenderClear(renderer);
    SDL_Rect dst = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
    SDL_RenderCopy(renderer, hostcodeTexture, NULL, &dst);
    SDL_RenderPresent(renderer);
}

////// Global Functions ///////

/*
Initializes SDL
*/
bool init_drawing()
{
    if(SDL_Init(SDL_INIT_VIDEO) < 0){
        fprintf(stderr, "Failed to initialize SDL\n");
        return false;
    }
    if((window = SDL_CreateWindow("PONG",
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        SCREEN_HEIGHT, SCREEN_WIDTH,
        SDL_WINDOW_SHOWN)) == NULL){
            fprintf(stderr,"Failed to create window\n");
            SDL_Quit();
            return true;
        }
    
    if((renderer = SDL_CreateRenderer(window, -1, 0)) == NULL){
        fprintf(stderr, "Failed to create renderer\n");
        SDL_Quit();
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        return true;
    }

    SDL_Surface* tempSurface = IMG_Load("./assets/background.png");
    if (!tempSurface) {
        fprintf(stderr, "Failed to load background: %s\n", IMG_GetError());
        SDL_Quit();
        return true;
    }

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (!backgroundTexture) {
        fprintf(stderr, "Failed to create background texture: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }

    SDL_Surface* homeSurface = IMG_Load("./assets/home_screen.png");
    if (!homeSurface) {
        fprintf(stderr, "Failed to load home screen image: %s\n", IMG_GetError());
        return true;
    }
    homeScreenTexture = SDL_CreateTextureFromSurface(renderer, homeSurface);
    SDL_FreeSurface(homeSurface);
    if (!homeScreenTexture) {
        fprintf(stderr, "Failed to create home screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }

    SDL_Surface* hostcodeSurface = IMG_Load("./assets/host_code.png");
    if (!hostcodeSurface) {
        fprintf(stderr, "Failed to load hostcode screen image: %s\n", IMG_GetError());
        return true;
    }

    hostcodeTexture = SDL_CreateTextureFromSurface(renderer, hostcodeSurface);
    SDL_FreeSurface(hostcodeSurface);
    if (!hostcodeTexture) {
        fprintf(stderr, "Failed to create hostcode screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }

    SDL_Surface* ballSurface = IMG_Load("./assets/Ball.png");
    if (!ballSurface) {
        fprintf(stderr, "Failed to load ball image: %s\n", IMG_GetError());
        return true;
    }
    ballTexture = SDL_CreateTextureFromSurface(renderer, ballSurface);
    SDL_FreeSurface(ballSurface);
    if (!ballTexture) {
        fprintf(stderr, "Failed to create ball texture: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }

    numbers = SDL_LoadBMP("./assets/numbers.bmp");
    if (numbers == NULL) {
        printf("Unable to load bitmap. Error: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }

    numTexture = SDL_CreateTextureFromSurface(renderer, numbers);
    if (numTexture == NULL) {
        printf("Unable to create texture from surface! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return true;
    }
    
    return true;
}

/*
Draw all the assets for playing the game
*/
void draw_playing(Paddle * paddles, Ball ball, int * score){

    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    SDL_Rect dst = {0, 0, SCREEN_HEIGHT, SCREEN_WIDTH};
    SDL_RenderCopy(renderer, backgroundTexture, NULL, &dst);
    draw_scores(renderer, numTexture, score);
    draw_ball(renderer, ballTexture, ball);
    draw_paddles(renderer, paddles);
    
    SDL_RenderPresent(renderer);

    //hard codes a delay
    SDL_Delay(10);
}

/*
Draw all assets for end screen
*/
void draw_endgame(){
    SDL_RenderClear(renderer);
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderPresent(renderer);
}

/*
Draw home screen
*/
void draw_homescreen(){
    draw_home_screen(renderer, homeScreenTexture);
}

/*
Draw host code screen
*/
void draw_hostcode(){
    hostcode_screen(renderer, hostcodeTexture);
}

/*
Destroy
*/
void destroy(){
    SDL_DestroyTexture(backgroundTexture);
    SDL_DestroyTexture(homeScreenTexture);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

