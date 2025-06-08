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
extern bool allBadgesEarned;

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

// ========== Unlock Final Badge ========== //
void checkFinalBadgeUnlock();

/**
IMPORTANT: HOW TO UNLOCK BADGES

    // Badges Implementation
    if (consecutiveWins >= 3 && !badgeProgress[1]) {
      badgeProgress[1] = true;
      isUnlocked[1] = true;
      saveBadgeProgress();

      // Add to notifications array for Main Menu
      hasPendingNotification = true;
      pendingNotificationMessage = "Pong Badge Unlocked!";
      pendingNotificationDuration = 3000;

*/