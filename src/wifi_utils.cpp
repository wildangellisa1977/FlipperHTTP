#include "wifi_utils.h"

bool WiFiUtils::connectHelper(const char *ssid, const char *password, bool isAP)
{
    if (isAP && strlen(ssid) == 0)
    {
        return false;
    }
    if (!isAP && (strlen(ssid) == 0 || strlen(password) == 0))
    {
        return false;
    }
#ifndef BOARD_BW16
    WiFi.disconnect(true);
#else
    WiFi.disconnect();
#endif
    if (isAP)
    {
#ifndef BOARD_BW16
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssid);
#else
        WiFi.apbegin((char *)ssid, "", "1");
#endif
    }
    else
    {
#ifndef BOARD_BW16
        WiFi.mode(WIFI_STA);
        WiFi.begin(ssid, password);
#else
        WiFi.begin((char *)ssid, password);
#endif
    }
#ifdef BOARD_ESP32_C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif
    if (!isAP)
    {
        int i = 0;
        while (!this->isConnected() && i < 20)
        {
            delay(500);
            i++;
        }
        return this->isConnected();
    }
    return true;
}

bool WiFiUtils::connect(const char *ssid, const char *password)
{
    if (this->connectHelper(ssid, password))
    // Set the time zone to UTC+0
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
    if (!this->connectHelper(ssid, "", true))
    {
        return "";
    }
#ifndef BOARD_BW16
    return WiFi.softAPIP().toString();
#else
    return WiFi.localIP().get_address();
#endif
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