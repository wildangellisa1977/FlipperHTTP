#include "wifi_utils.h"

bool WiFiUtils::connect(const char *ssid, const char *password)
{
    // Check if the provided SSID and password are not empty
    if (strlen(ssid) == 0 || strlen(password) == 0)
    {
        return false;
    }

    // Ensure WiFi is disconnected attempting to connect
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    WiFi.disconnect();
#endif
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
#ifdef BOARD_ESP32_C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
    // Attempt to connect to WiFi
    int i = 0;
    while (!this->isConnected() && i < 20)
    {
        delay(500);
        i++;
    }

    // return connection status and get UTC time via NTP
    if (this->isConnected())
    {
#ifndef BOARD_BW16
        configTime(0, 0, "pool.ntp.org", "time.nist.gov");
#endif
        return true;
    }
    return false;
}

String WiFiUtils::connectAP(const char *ssid)
{
    // Check if the provided SSID is not empty
    if (strlen(ssid) == 0)
    {
        return "";
    }

    // Ensure WiFi is disconnected attempting to connect
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    WiFi.disconnect();
#endif
    WiFi.mode(WIFI_AP);
    WiFi.softAP(ssid);
#ifdef BOARD_ESP32_C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
    return WiFi.softAPIP().toString();
}

String WiFiUtils::device_ip()
{
#ifndef BOARD_BW16
    return WiFi.localIP().toString();
#else
    return WiFi.localIP().get_address();
#endif
}

void WiFiUtils::disconnect()
{
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    WiFi.disconnect();
#endif
}

bool WiFiUtils::isConnected()
{
    return WiFi.status() == WL_CONNECTED;
}

String WiFiUtils::scan()
{
    int n = WiFi.scanNetworks();
    String json = "{\"networks\":[";
    for (int i = 0; i < n; ++i)
    {
        json += "\"";
        json += WiFi.SSID(i);
        json += "\"";
        if (i < n - 1)
        {
            json += ",";
        }
    }
    json += "]}";
    return json;
}