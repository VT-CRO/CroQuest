// /src/JoinHost/JoinHost.cpp#include "JoinHost.hpp"

#include "JoinHost.hpp"

extern TFT_eSPI tft; // assuming you already declared this globally

bool JoinHost::attemptJoinGame(const std::string &code) {
  ConnectionScreen::init(tft);
  ConnectionScreen::showMessage("Searching for host...\nCode: " +
                                String(code.c_str()));

  BluetoothManager::initCentral(tft);
  BluetoothCentral &central = BluetoothManager::getCentral();

  central.beginScan(code);
  delay(5000);
  central.connectToDevices();

  if (!central.getConnectedClients().empty()) {
    return true;
  } else {
    ConnectionScreen::showMessage("No host found.\nCheck your code.");
    delay(2000);
    return false;
  }
}
