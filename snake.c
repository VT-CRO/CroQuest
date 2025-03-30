#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <unistd.h>

// Creating the board width and height
// Easy to change once we have sizes confirmed
#define BOARD_WIDTH 20
#define BOARD_HEIGHT 20

// Directional functionality
// Don't know how much this will change, can be a lot
#define UP 0
#define DOWN 1
#define LEFT 2
#define RIGHT 3

typedef struct
{
    int x;
    int y;
} Location;

// How big the snake can grow?
Location snake[100];

// Starting off with a length of 1
int s_length = 1;

// Starting with the direction moving right
int dir = RIGHT;

// Food
Location food;

// Creating the game
void init_snake()
{
    // Snake starting location
    snake[0].x = BOARD_WIDTH / 2;
    snake[0].x = BOARD_HEIGHT / 2;
    food.x = rand() % BOARD_WIDTH;
    food.y = rand() % BOARD_HEIGHT;
}