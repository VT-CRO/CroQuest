#pragma once
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, B, up, down, left, right;

// ============ API ==============
void runSnake();