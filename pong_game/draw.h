#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_main.h>

bool init_drawing();
void draw_playing(Paddle * paddles, Ball ball, int * score);
void draw_endgame();
void draw_homescreen();
void draw_hostcode();
void destroy();