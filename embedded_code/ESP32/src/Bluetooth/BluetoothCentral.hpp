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
#include "Games/tic_tac_toe/TicTacToe.hpp"

class BluetoothCentral {
public:
  volatile bool scanComplete;
  BluetoothCentral(TFT_eSPI &display);

  void onScanComplete(const NimBLEScanResults& results, int reason);

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

  // ###################### Get Connected Clients #####################
  const std::vector<NimBLEClient *> &getConnectedClients() const;

private:
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
    void onScanEnd(const NimBLEScanResults& results, int reason) override;
  private:
    BluetoothCentral *parent;
  };
};
