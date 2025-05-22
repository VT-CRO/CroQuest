// src/Bluetooth/BluetoothPeripheral.hpp

#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <TFT_eSPI.h>
#include <string>

#include "BluetoothCommon.hpp"
#include "ConnectionScreen.hpp"

class BluetoothPeripheral {
public:
  BluetoothPeripheral(TFT_eSPI &display);

  // ###################### Start Advertising #####################
  void beginAdvertising(const std::string &accessCode);

  // ###################### Update Data #####################
  void update();

  // ###################### Voicemail #####################
  void
  setResponseHandler(std::function<std::string(const std::string &)> handler);

private:
  TFT_eSPI &tft;
  NimBLEServer *server = nullptr;
  NimBLECharacteristic *characteristic = nullptr;
  NimBLEAdvertising *advertising = nullptr;
  std::string accessCode;

  std::string lastHostMessage = "";
  std::string lastReply = "";

  std::function<std::string(const std::string &)> responseHandler;

  // ####################################################################################################
  //  Server Callbacks
  // ####################################################################################################
  class ServerCallbacks : public NimBLEServerCallbacks {
  public:
    ServerCallbacks(BluetoothPeripheral *parent) : parent(parent) {}

    // ###################### Connect #####################
    void onConnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo) override;

    // ###################### Disconnect #####################
    void onDisconnect(NimBLEServer *pServer, NimBLEConnInfo &connInfo,
                      int reason) override;

  private:
    BluetoothPeripheral *parent;
  };
};
