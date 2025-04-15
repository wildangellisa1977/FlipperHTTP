#pragma once
#include <Arduino.h>
#include "boards.h"
#ifndef BOARD_BW16
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#else
#include "WiFiClient.h"
#include "WiFiServer.h"
#endif
#include "WiFi.h"

class WiFiUtils
{
public:
    WiFiUtils()
    {
    }
    bool connect(const char *ssid, const char *password); // Connect to WiFi using the provided SSID and password
    String connectAP(const char *ssid);                   // Connect to WiFi in AP mode and return the IP address
    String deviceIP();                                    // Get IP address of the device
    void disconnect();                                    // Disconnect from WiFi
    bool isConnected();                                   // Check if connected to WiFi
    String scan();                                        // Scan for available WiFi networks
private:
    bool connectHelper(const char *ssid, const char *password, bool isAP = false); // Helper function to connect to WiFi
};