/* FlipperHTTP.h for flipper-http.ino
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-09-30
Updated: 2025-03-11

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
    - Created an UART class to handle Serial actions for all supported boards
    - Combined source files from all supported boards into one file
- 2025-02-03: Added WebSockets support: [SOCKET/START] and [SOCKET/STOP]
- 2025-02-16: Added support for the ESP32-C3 board (@dj1ch)
- 2025-02-21: Simplified HTTP requests
- 2025-02-22: Update the request response to include the status code and content length
- 2025-03-11: Updated certs and started support for the BW16 board (storage.cpp is left)
*/
#pragma once
#include "certs.h"
#include "led.h"
#include "uart.h"
#ifndef BOARD_BW16
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#else
#include "WiFiClient.h"
#include "WiFiServer.h"
#endif
#include "WiFi.h"
#include <ArduinoJson.h>
#include <Arduino.h>
#include <ArduinoHttpClient.h>
#include <stdint.h>
#include <string.h>

#define BAUD_RATE 115200

class FlipperHTTP
{
public:
    // Constructor
    FlipperHTTP()
    {
    }

    void clearSerialBuffer(); // Clear serial buffer to avoid any residual data
    bool connectToWifi();     // Connect to Wifi using the loaded SSID and Password
#ifndef BOARD_BW16
    String getIPAddress() { return WiFi.localIP().toString(); } // get IP addresss
#else
    String getIPAddress() { return WiFi.localIP().get_address(); } // get IP addresss
#endif
    bool isConnectedToWifi() { return WiFi.status() == WL_CONNECTED; } // Check if the Dev Board is connected to Wifi
    bool loadWifiSettings();                                           // Load Wifi settings from storage
    //
    String request(
        const char *method,                   // HTTP method
        String url,                           // URL to send the request to
        String payload = "",                  // Payload to send with the request
        const char *headerKeys[] = nullptr,   // Array of header keys
        const char *headerValues[] = nullptr, // Array of header values
        int headerSize = 0                    // Number of headers
    );
    //
    bool saveWifiSettings(String data);                                                                                                      // Save and Load settings to and from storage
    String scanWifiNetworks();                                                                                                               // returns a string of all wifi networks
    void setup();                                                                                                                            // Arduino setup function
    bool stream_bytes(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize); // Stream bytes from server
    bool read_serial_settings(String receivedData, bool connectAfterSave);                                                                   // Read the serial data and save the settings
    bool upload_bytes(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);                     // stream bytes to server

    void loop(); // Main loop for flipper-http.ino that handles all of the commands
private:
    char loadedSSID[64] = {0};     // Variable to store SSID
    char loadedPassword[64] = {0}; // Variable to store password
    bool useLED = true;            // Variable to control LED usage
#ifndef BOARD_BW16
    WiFiClientSecure client; // WiFiClientSecure object for secure connections
#else
    WiFiSSLClient client; // WiFiClient object for secure connections
#endif
    LED led; // EasyLED object to control the LED

#ifdef BOARD_VGM
    UART uart;   // UART object to handle serial communication
    UART uart_2; // UART object to handle serial communication
#else
    UART uart; // UART object to handle serial communication
#endif
};

const PROGMEM char settingsFilePath[] = "/flipper-http.json"; // Path to the settings file in the SPIFFS file system
