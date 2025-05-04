#pragma once

#include <Arduino.h>
#include <SPI.h>
#include <SD.h>
#include <TFT_eSPI.h>

#define UP 22
#define DOWN 23
#define LEFT 17
#define RIGHT 16
#define A 21 
#define B 19
#define HOST_CODE_SIZE 6

typedef struct {
int y;
bool paddle_mod;
} Prev_Paddle;