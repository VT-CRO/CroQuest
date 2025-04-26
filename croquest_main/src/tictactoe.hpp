// tictactoe.hpp
#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

// Global state (defined once in tictactoe.cpp)
extern uint64_t last_ms;
extern int      random_number;

/**
 * Update/paint the Tic-Tac-Toe diagnostic screen.
 * Call once from your main loop.
 */
void tictactoe(TFT_eSPI* tft);
