#pragma once
#include <Arduino.h>
#include "wifi_utils.h"
#include "uart.h"

class WiFiAP
{
public:
    WiFiAP(UART *uartClass, WiFiUtils *wifiUtils);
    bool start(const char *ssid);
    void run();

private:
    void updateHTML(String html);
    UART *uart;      // UART object for serial communication
    WiFiUtils *wifi; // WiFiUtils object for WiFi operations
    bool isRunning;  // Flag to indicate if the AP mode is running
    String html;     // HTML content to be served
    String ip;       // IP address to connect to in AP mode
};
