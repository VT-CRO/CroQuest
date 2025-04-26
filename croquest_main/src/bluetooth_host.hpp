#pragma once

#include <Arduino.h>
#include <vector>

// Begins looking for all bluetooth devices that are advertising with the given host code.
// quest-guest-xxx-yyyyyy where x is the host code and y is random (users will never see y)
void                btScanAndConnect(uint16_t host_code);

// returns a list of all connected devices to the host. dynamically updated.
std::vector<String> btConnectedDevices();

// sends a string to all guest devices.
// will always start with host:{string}
bool                btSendToGuests(const String& data);

// receives a string from the guest devices.
// recieved strings will always start with quest-guest-xxx-yyyyyy:{string}
std::vector<String> btReceivedFromGuest();