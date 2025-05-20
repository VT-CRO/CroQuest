// BluetoothHost.hpp

#pragma once

#include <Arduino.h>
#include <NimBLEDevice.h>
#include <TFT_eSPI.h>

class HostBLEServer {
public:
  HostBLEServer(TFT_eSPI &display); // ✅ Just declare the constructor here
  void begin();
  void update();

private:
  void startScan();
  void connectToDevice();

  TFT_eSPI &tft;
  bool foundDevice = false;
  NimBLEAdvertisedDevice *matchedDevice = nullptr;
  const std::string targetCode = "code:060970";

  class MyScanCallbacks : public NimBLEScanCallbacks {
  public:
    MyScanCallbacks(HostBLEServer *host) : parent(host) {}
    void onResult(const NimBLEAdvertisedDevice *advertisedDevice) override;

  private:
    HostBLEServer *parent;
  };
};
