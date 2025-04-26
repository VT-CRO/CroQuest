#pragma once

#include <Arduino.h>
#include <vector>

// When a host code is typed in, it begins advertising itself with quest-guest-xxx-yyyyyy.
void                btStartAdvertising(uint8_t host_code); 
void                btStopAdvertising(); 

// Returns if the device is connected to a host.
bool                btIsConnected();

// Sends data to a host device. The data will always start with quest-guest-xxx-yyyyyy:{string}
bool                btSendToHost(const String& data);

// Receives data from the host device. The data will always start with host:{string}
std::vector<String> btReceivedFromHost();