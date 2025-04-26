#ifndef BLUETOOTH_H
#define BLUETOOTH_H

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <Arduino.h>

// Callback function types for game events
typedef void (*PaddleUpdateCallback)(int player, int y);
typedef void (*BallUpdateCallback)(double x, double y, double speedX, double speedY);
typedef void (*ScoreUpdateCallback)(int player);

// Initialize BLE functionality
void initializeBluetooth();

// Setup device as a host
void setupHost();

// Connect to a host
bool connectToHost();

// Start scanning for hosts
void startScan();

// Send paddle position updates
void sendPaddlePosition(int player, int y);

// Send ball position and speed updates
void sendBallPosition(double x, double y, double speedX, double speedY);

// Send score updates
void sendScore(int player);

// Send player ready status
void sendReadyStatus(bool isHost);

// Check connection status
bool isDeviceConnected();

// Start/stop advertising
void startAdvertising();
void stopAdvertising();

// Set connection code
void setConnectionCode(String code);

// Get connection code
String getHostCode();

// Set host mode
void setHostMode(bool isHostMode);

// Check if in host mode
bool isInHostMode();

// Get player ready status
bool isHostReady();
bool isClientReady();

// Reset ready status
void resetReadyStatus();

// Enable/disable multiplayer mode
void enableMultiplayer();
void disableMultiplayer();

// Check if multiplayer is enabled
bool isMultiplayerEnabled();

// Register callback functions
void registerPaddleUpdateCallback(PaddleUpdateCallback callback);
void registerBallUpdateCallback(BallUpdateCallback callback);
void registerScoreUpdateCallback(ScoreUpdateCallback callback);

#endif // BLUETOOTH_H