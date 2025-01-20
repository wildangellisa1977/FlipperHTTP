/* FlipperHTTP.h for flipper-http.ino
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2025-01-19

Change Log:
- 2024-09-30: Initial commit
.
.
.
- 2024-10-16: Fixed typos and added [GET/BYTES], [POST/BYTES], and [WIFI/SACN] commands
- 2024-10-17: Added [LIST], [REBOOT], [PARSE], [PARSE/ARRAY], [LED/ON], and [LED/OFF], and [IP/ADDRESS] commands
- 2024-10-19: Added [WIFI/IP] command
- 2024-10-21: Removed unnecessary println
- 2024-10-22: Updated Post Bytes and Get Bytes methods
- 2024-10-25: Updated to automatically connect to WiFi on boot if settings are saved
- 2024-10-26: Updated the saveWifiSettings and loadWifiSettings methods to save and load a list of wifi networks, and added [WIFI/LIST] command
- 2024-11-09: Added SSL certificate from https://letsencrypt.org/certificates/
- 2024-12-02: Restructured the code
- 2025-01-12:
    - Added uploadBytes method
    - Added timeout to stream requests
    - Added NTP time sync
- 2025-01-19:
    - Created a LED class to handle LED actions for all supported boards
    - Combined source files from all supported boards into one file
*/
#pragma once
#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include "led.h"
#include "uart.h"

#define BAUD_RATE 115200

class FlipperHTTP
{
public:
    // Constructor
    FlipperHTTP()
    {
    }

    void clearSerialBuffer();                                                                                                 // Clear serial buffer to avoid any residual data
    bool connectToWifi();                                                                                                     // Connect to Wifi using the loaded SSID and Password
    String delete_request(String url, String payload);                                                                        // Delete request
    String delete_request(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);  // Delete request with headers
    String get(String url);                                                                                                   // Get request
    String get(String url, const char *headerKeys[], const char *headerValues[], int headerSize);                             // Get request with headers
    String getIPAddress() { return WiFi.localIP().toString(); }                                                               // get IP addresss
    bool isConnectedToWifi() { return WiFi.status() == WL_CONNECTED; }                                                        // Check if the Dev Board is connected to Wifi
    bool loadWifiSettings();                                                                                                  // Load Wifi settings from storage
    String post(String url, String payload);                                                                                  // Post request
    String post(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);            // Post request with headers
    String put(String url, String payload);                                                                                   // Put request
    String put(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);             // Put request with headers
    bool saveWifiSettings(String data);                                                                                       // Save and Load settings to and from storage
    String scanWifiNetworks();                                                                                                // returns a string of all wifi networks
    void setup();                                                                                                             // Arduino setup function
    bool stream_bytes_get(String url, const char *headerKeys[], const char *headerValues[], int headerSize);                  // Get request to process bytes
    bool stream_bytes_post(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize); // Post request to process bytes
    String readSerialLine();                                                                                                  // Read serial data until newline character
    bool read_serial_settings(String receivedData, bool connectAfterSave);                                                    // Read the serial data and save the settings
    bool upload_bytes(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);      // Upload bytes to server

    void loop(); // Main loop for flipper-http.ino that handles all of the commands
private:
    const char *settingsFilePath = "/flipper-http.json"; // Path to the settings file in the SPIFFS file system
    char loadedSSID[64] = {0};                           // Variable to store SSID
    char loadedPassword[64] = {0};                       // Variable to store password
    bool useLED = true;                                  // Variable to control LED usage
    WiFiClientSecure client;                             // WiFiClientSecure object for secure connections
    LED led;                                             // EasyLED object to control the LED

#ifdef BOARD_VGM
    UART uart;   // UART object to handle serial communication
    UART uart_2; // UART object to handle serial communication
#else
    UART uart; // UART object to handle serial communication
#endif

    // Root CA from letsencrypt
    // get it here: https://letsencrypt.org/certificates/
    const char *root_ca =
        "-----BEGIN CERTIFICATE-----\n"
        "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\n"
        "-----END CERTIFICATE-----\n";
};