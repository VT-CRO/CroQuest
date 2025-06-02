// Games/chss/Chess.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "HostGame/HostGame.hpp"
#include "JoinHost/JoinHost.hpp"
#include "Menu/MenuReturn.hpp"
#include "NumPad/NumPad.hpp"
#include <TFT_eSPI.h>

// ========== API ==========
void runChess();
void handleChessFrame();

// ========== Globals ==========
extern TFT_eSPI tft;
extern JpegDrawing drawing;
extern Button A, B, up, down, left, right;