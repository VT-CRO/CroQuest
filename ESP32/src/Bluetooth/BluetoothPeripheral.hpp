// src/Bluetooth/BluetoothPeripheral.hpp

#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <TFT_eSPI.h>
#include <string>

#include "BluetoothCommon.hpp"
#include "ConnectionScreen.hpp"

// Games
#include "Games/connect4/Connect4.hpp"
#include "Games/tic_tac_toe/TicTacToe.hpp"

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

  // ###################### Send Messages #####################
  bool sendAction(const std::string &message);

  // ###################### Read Messages #####################
  std::string readMessage(); // Expose message reading

  // ###################### Send Messages #####################
  void sendMessage(const std::string &message); // Alias to sendAction
  void sendExit();                              // Sends exit message

  NimBLEServer *server = nullptr;

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
    bool intentionalExit = false;

  private:
    BluetoothPeripheral *parent;
  };

  ServerCallbacks *callbackServer = nullptr;

private:
  TFT_eSPI &tft;
  NimBLECharacteristic *characteristic = nullptr;
  NimBLEAdvertising *advertising = nullptr;
  std::string accessCode;

  std::string lastHostMessage = "";
  std::string lastReply = "";

  std::function<std::string(const std::string &)> responseHandler;

  class CharacteristicCallbacks : public NimBLECharacteristicCallbacks {
  public:
    CharacteristicCallbacks(ServerCallbacks *server) : server(server) {}
    void onWrite(NimBLECharacteristic *pCharacteristic,
                 NimBLEConnInfo &connInfo) override;

  private:
    ServerCallbacks *server;
  };
};
