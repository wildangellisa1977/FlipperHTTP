#pragma once
#include <Arduino.h>
#include "boards.h"
#ifndef BOARD_BW16
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#else
#include "WiFiClient.h"
#include "WiFiServer.h"
#include <wifi_conf.h>
#define WIFI_STA RTW_MODE_STA
#define WIFI_AP RTW_MODE_AP

typedef struct
{
    String ssid;
    String bssid_str;
    uint8_t bssid[6];
    short rssi;
    uint8_t channel;
    rtw_security_t security;
} WiFiScanResult;

// Maximum number of APs to record in a single scan
#define BW16_MAX_SCAN 32

// Global scan results array and count
extern WiFiScanResult bw16ScanResults[BW16_MAX_SCAN];
extern int bw16_scan_count;
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