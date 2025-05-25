#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== Globals ==========
extern Button A, B, up, down, left, right;
extern JpegDrawing drawing;
extern TFT_eSPI tft;

// ========== API ==========
void runTetris();