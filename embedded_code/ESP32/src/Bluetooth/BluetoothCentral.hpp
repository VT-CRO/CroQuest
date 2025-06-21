// src/Bluetooth/BluetoothCentral.hpp

#pragma once

#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <map>
#include <string>
#include <vector>

#include "BluetoothCommon.hpp"
#include "BluetoothManager.hpp"
#include "ConnectionScreen.hpp"
#include "Core/Buttons.hpp"
#include "Menu/MenuReturn.hpp"

// Include games
#include "Games/connect4/Connect4.hpp"
#include "Games/tic_tac_toe/TicTacToe.hpp"

class BluetoothCentral {
public:
  BluetoothCentral(TFT_eSPI &display);

  // ###################### Start Scanning #####################
  void beginScan(const std::string &accessCode);

  // ###################### Keep Scanning for more Players #####################
  void scanAndConnectLoop(const std::string &accessCode);

  // ###################### Connect to All Devices #####################
  void connectToDevices();

  // ###################### Poll Devices for Incoming Data #####################
  void pollDevices();

  // #################### Send Message to Specific Device ###################
  bool sendToDevice(NimBLEClient *client, const std::string &message);

  // ###################### Disconnect from ALL Devices #####################
  void disconnectAll();

  // ###################### Update Poll #####################
  void update();

  // ###################### Send Messages (not array) #####################
  bool sendMessage(const std::string &msg);
  bool sendMessagePong(const std::string &msg);
  bool sendExit();
  // ###################### Read Messages #####################
  std::string readMessage();

  // ###################### Get Connected Clients #####################
  const std::vector<NimBLEClient *> &getConnectedClients() const;

  // setter used to exit host screen
  void setHostScreenExit(bool exit);

  class MyClientCallbacks : public NimBLEClientCallbacks {
  public: 
    void onDisconnect(NimBLEClient *client, int reason) override;
    bool intentionalExit = false;
  };

  MyClientCallbacks * callbacks = nullptr;

private:
  bool hostScreenExit = false;
  std::string latestMessage = ""; // Latest message sent
  TFT_eSPI &tft;
  std::string targetCode;

  std::vector<NimBLEAdvertisedDevice> foundDevices;
  std::vector<NimBLEClient *> connectedClients;

  std::map<NimBLEClient *, std::string> lastMessages;
  std::map<NimBLEClient *, std::string> lastReplies;

  std::string sanitize(const std::string &input);

  // ####################################################################################################
  //  Scan Callbacks
  // ####################################################################################################
  class ScanCallbacks : public NimBLEScanCallbacks {
  public:
    ScanCallbacks(BluetoothCentral *parent) : parent(parent) {}
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

  private:
    BluetoothCentral *parent;
  };
};
