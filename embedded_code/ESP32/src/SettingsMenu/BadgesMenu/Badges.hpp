// src/SettingsMenu/BadgesMenu/Badges.hpp

#pragma once

#include "Core/Buttons.hpp"
#include "Core/JpegDrawing.hpp"
#include "Notifications/Notification.hpp"
#include "SettingsMenu/AudioMenu/Audio.hpp"
#include "SettingsMenu/Settings/Settings.hpp"

const int badgeCount = 10;

extern bool badgeProgress[badgeCount];
extern bool isUnlocked[badgeCount];

// ========== Run Badges menu ========== //
void runBadgesMenu();

// ========== Draw the Badges ========== //
void drawBadges(int selectedIndex, int xOffset, int yOffset, int extraWidth,
                int extraHeight, int descX, int descY, int descW, int descH);

// ========== Draw Selector ========== //
void drawSelectorAndDescription(int index, int prevIndex, int xOffset,
                                int yOffset, int extraWidth, int extraHeight,
                                int descX, int descY, int descW, int descH);

// ========== Load Badges ========== //
void loadBadgeProgress();

// ========== Saves New Badge ========== //
void saveBadgeProgress();

// ========== Reset Badges ========== //
void resetBadgeProgress();

/**
IMPORTANT: HOW TO UNLOCK BADGES

    if (score >= 150 && !badgeProgress[X]) {
      badgeProgress[X] = true;
      isUnlocked[X] = true;
      saveBadgeProgress();

      triggerNotification("{Game} Badge Unlocked!", 3000);

*/