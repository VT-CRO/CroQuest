#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_main.h>

bool init_drawing();
void draw_playing(Paddle * paddles, Ball ball, int * score);
void draw_endgame();
void draw_homescreen();
void draw_hostcode();
void destroy();
void draw_button_hover(int x, int y);
void draw_button_pressed(int x, int y);
void init_buttons();
int get_button_pressed(int x, int y);
void draw_numbers(int* code, int code_size);

typedef struct {
    int x, y;
    int w, h;
    int button;
    SDL_Texture * hover;
    SDL_Texture * pressed;
    SDL_Texture * default_texture;
} Button;