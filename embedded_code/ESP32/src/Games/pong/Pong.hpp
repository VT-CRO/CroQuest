#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

#define HOST_CODE_SIZE 6

// ========== Globals ==========
extern TFT_eSPI tft;
extern Button A, B, up, down, left, right;

//============ API ==============
void runPong();