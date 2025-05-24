#pragma once 

#include "SettingsMenu/Settings/Settings.hpp"
// #include "Boot/Boot.hpp"
#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include <TFT_eSPI.h>

#define SPEAKER_PIN 21   // Speaker GPIO pin
int const channel = 0;   // Channel

// ================= API ================ //
void runAudioMenu();

// NOTE: Needs to be called anytime sound is used
void playTone(int toneFreq, int volume);

// Sounds
void playSelectBeep();
void playPressSound();
void backAudio();

// ============= VOLUME ================ //
extern int& volume;