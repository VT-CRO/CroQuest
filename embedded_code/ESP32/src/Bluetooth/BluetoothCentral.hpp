// src/Bluetooth/BluetoothCentral.hpp

#pragma once

#include <NimBLEAdvertisedDevice.h>
#include <NimBLEDevice.h>
#include <string>
#include <vector>

#include "BluetoothCommon.hpp"
#include "BluetoothManager.hpp"
#include "ConnectionScreen.hpp"

class BluetoothCentral {
public:
  BluetoothCentral(TFT_eSPI &display);

  // ###################### Start Scanning #####################
  void beginScan(const std::string &accessCode);

  // ###################### Keep Scanning for more Players #####################
  void scanAndConnectLoop();

  // ###################### Connect to All Devices #####################
  void connectToDevices();

  // ###################### Poll Devices for Incoming Data #####################
  void pollDevices();

  // #################### Send Message to Specific Device ###################
  void sendToDevice(NimBLEClient *client, const std::string &message);

  // ###################### Disconnect from ALL Devices #####################
  void disconnectAll();

private:
  TFT_eSPI &tft;
  std::string targetCode;

  std::vector<NimBLEAdvertisedDevice> foundDevices;
  std::vector<NimBLEClient *> connectedClients;

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
