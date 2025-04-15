#include <stdbool.h>
#include <SDL2/SDL_ttf.h>
#include "pong.h"
#include "draw.h"

#define NUM_WIDTH 80
#define NUM_PAD_WIDTH 3
#define NUM_PAD_LENGTH 4

#define BUTTON_WIDTH SCREEN_HEIGHT/7
#define BUTTON_HEIGHT SCREEN_WIDTH/7
#define BUTTON_SPACING 0

#define BUTTON_START_X ((SCREEN_HEIGHT - (NUM_PAD_WIDTH * BUTTON_WIDTH + (NUM_PAD_WIDTH - 1) * BUTTON_SPACING)) / 2)

#define BUTTON_START_Y (SCREEN_WIDTH * 0.7 - (NUM_PAD_LENGTH * BUTTON_HEIGHT + (NUM_PAD_LENGTH - 1) * BUTTON_SPACING) / 2)


static SDL_Window * window;
static SDL_Renderer* renderer;
static SDL_Texture* backgroundTexture;
static SDL_Texture * hostcodeTexture;
static SDL_Texture* homeScreenTexture;
static SDL_Texture* ballTexture;
static SDL_Surface* numbers;
static SDL_Texture* numTexture;
static TTF_Font* font;

static Button pad[NUM_PAD_WIDTH][NUM_PAD_LENGTH];


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
    for (int row = 0; row < NUM_PAD_LENGTH; row++) {
        for (int col = 0; col < NUM_PAD_WIDTH; col++) {
            Button b = pad[col][row];
            SDL_Rect dest = {b.x, b.y, b.w, b.h};
            
            SDL_RenderCopy(renderer, b.default_texture, NULL, &dest);
        }
    }
}

/*
Loading button images
*/
SDL_Texture* load_button_texture(SDL_Renderer* renderer, const char* base, const char* state) {
    char filename[128];
    snprintf(filename, sizeof(filename), "assets/btn_%s_%s.png", base, state);
    
    SDL_Surface* surface = IMG_Load(filename);
    if (!surface) {
        fprintf(stderr, "Failed to load button image %s: %s\n", filename, IMG_GetError());
        return NULL;
    }

    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_FreeSurface(surface);  // Free the surface after converting to texture

    if (!texture) {
        fprintf(stderr, "Failed to create texture from %s: %s\n", filename, SDL_GetError());
    }

    return texture;
}

/*
Initializes all buttons
*/
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

            char base[16];
            if (b->button == -1) {
                strcpy(base, "enter");
            } else if (b->button == -2) {
                strcpy(base, "del");
            } else {
                snprintf(base, sizeof(base), "%d", b->button);
            }

            b->hover = load_button_texture(renderer, base, "hover");
            b->pressed = load_button_texture(renderer, base, "pressed");
            b->default_texture = load_button_texture(renderer, base, "default");
        }
    }
}

void draw_button_hover(int x, int y) {
    Button* b = &pad[x][y];
    SDL_Rect dest = {b->x, b->y, b->w, b->h};
    SDL_RenderCopy(renderer, b->hover, NULL, &dest);
}

void draw_button_pressed(int x, int y) {
    Button* b = &pad[x][y];
    SDL_Rect dest = {b->x, b->y, b->w, b->h};
    SDL_RenderCopy(renderer, b->pressed, NULL, &dest);
    SDL_RenderPresent(renderer);  
}

void draw_numbers(int* code, int code_size) {
    float scale_factor = SCREEN_HEIGHT / 400.0f;
    
    SDL_Rect sourceRect = {0, 0, NUM_WIDTH, numbers->h};
    SDL_Rect destRect = {0, BUTTON_START_Y * 0.8 - ((numbers->h / 3) * scale_factor), (NUM_WIDTH / 3) * scale_factor, (numbers->h / 3) * scale_factor};
    
    for(int i = 0; i < code_size; i++) {
        sourceRect.x = code[i] * NUM_WIDTH;
        destRect.x = ((SCREEN_HEIGHT / 2) - (3*(NUM_WIDTH/3)*scale_factor)) + i * (NUM_WIDTH / 3)* scale_factor;
        SDL_RenderCopy(renderer, numTexture, &sourceRect, &destRect);
    }

    char* headerText = "Join a Host Code";
    SDL_Color textColor = {255, 255, 255, 255}; // White text
    
    // Create text surface and texture
    SDL_Surface* textSurface = TTF_RenderText_Solid(font, headerText, textColor);
    if (textSurface) {
        SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
        
        if (textTexture) {
            // Calculate position for text (centered horizontally, above the digits)
            SDL_Rect textRect = {0, 0, textSurface->w, textSurface->h};
            textRect.x = (SCREEN_HEIGHT - textSurface->h) / 2 - 2*textSurface->h;
            textRect.y = BUTTON_START_Y * 0.8 - ((numbers->h / 3) * scale_factor) - textSurface->h - 10; // 10 pixels above the digits
            
            // Render the text
            SDL_RenderCopy(renderer, textTexture, NULL, &textRect);
            SDL_DestroyTexture(textTexture);
        }
        SDL_FreeSurface(textSurface);
    }

    SDL_RenderPresent(renderer);
}

int get_button_pressed(int x, int y){
    return pad[x][y].button;
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
            return false;
        }
    
    if((renderer = SDL_CreateRenderer(window, -1, 0)) == NULL){
        fprintf(stderr, "Failed to create renderer\n");
        SDL_Quit();
        SDL_DestroyWindow(window);
        SDL_DestroyRenderer(renderer);
        return false;
    }

    SDL_Surface* tempSurface = IMG_Load("./assets/background.png");
    if (!tempSurface) {
        fprintf(stderr, "Failed to load background: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }

    backgroundTexture = SDL_CreateTextureFromSurface(renderer, tempSurface);
    SDL_FreeSurface(tempSurface);
    if (!backgroundTexture) {
        fprintf(stderr, "Failed to create background texture: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    SDL_Surface* homeSurface = IMG_Load("./assets/home_screen.png");
    if (!homeSurface) {
        fprintf(stderr, "Failed to load home screen image: %s\n", IMG_GetError());
        return false;
    }
    homeScreenTexture = SDL_CreateTextureFromSurface(renderer, homeSurface);
    SDL_FreeSurface(homeSurface);
    if (!homeScreenTexture) {
        fprintf(stderr, "Failed to create home screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    SDL_Surface* hostcodeSurface = IMG_Load("./assets/host_code.png");
    if (!hostcodeSurface) {
        fprintf(stderr, "Failed to load hostcode screen image: %s\n", IMG_GetError());
        return false;
    }

    hostcodeTexture = SDL_CreateTextureFromSurface(renderer, hostcodeSurface);
    SDL_FreeSurface(hostcodeSurface);
    if (!hostcodeTexture) {
        fprintf(stderr, "Failed to create hostcode screen texture: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    SDL_Surface* ballSurface = IMG_Load("./assets/Ball.png");
    if (!ballSurface) {
        fprintf(stderr, "Failed to load ball image: %s\n", IMG_GetError());
        return false;
    }
    ballTexture = SDL_CreateTextureFromSurface(renderer, ballSurface);
    SDL_FreeSurface(ballSurface);
    if (!ballTexture) {
        fprintf(stderr, "Failed to create ball texture: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    numbers = SDL_LoadBMP("./assets/numbers.bmp");
    if (numbers == NULL) {
        printf("Unable to load bitmap. Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    numTexture = SDL_CreateTextureFromSurface(renderer, numbers);
    if (numTexture == NULL) {
        printf("Unable to create texture from surface! SDL_Error: %s\n", SDL_GetError());
        SDL_Quit();
        return false;
    }

    TTF_Init();
    font = TTF_OpenFont("assets/font.ttf", 16);
    if (!font) {
        fprintf(stderr, "Failed to load font: %s\n", TTF_GetError());
        SDL_Quit();
        return false;
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

