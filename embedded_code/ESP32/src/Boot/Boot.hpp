#pragma once

#include <Arduino.h>
#include <TFT_eSPI.h>

// ======================== Constants ========================
#define STEPS 15         // steps for fading
#define DELAY_MS 150     // delay between fade frames
#define CIRCLE_RADIUS 45 // circle radius for boot animation
#define SPACING 97       // spacing between circles
#define SPEAKER_PIN 21   // Speaker GPIO pin

// ======================== Display ==========================
extern TFT_eSPI tft; // Declare shared display instance

// ======================== Boot Animation API ==========================
void initBoot();                            // Initializes TFT + SD
void showBootWithLoading(const char *path); // Full boot sequence

// Internal helpers
void drawSdJpeg(const char *filename, int x, int y); // Draws a JPEG from SD
void jpegRender(int x, int y);                // Renders JPEG from JpegDec
void fadeInJpeg();                            // Simple fade-in effect
void drawCroQuestCircleGrid(float progress);  // Grid circle animation frame
void showLoadingDotsLine(int frame);          // Animated dots for "Loading..."
void showBootLoadingCombined(int durationMs); // Combined loading effect
void speaker();
