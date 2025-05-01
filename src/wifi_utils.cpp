#include "wifi_utils.h"

#ifdef BOARD_BW16
#include <wifi_conf.h>
#define WIFI_STA RTW_MODE_STA
#define WIFI_AP RTW_MODE_AP
#endif

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

#ifdef BOARD_ESP32_C5
    WiFi.setBandMode(WIFI_BAND_MODE_AUTO);
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

String WiFiUtils::deviceIP()
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

/* To use later
#ifdef BOARD_BW16
#include <wifi_conf.h>

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
  static constexpr int BW16_MAX_SCAN = 32;

  // Global scan results array and count
  WiFiScanResult bw16_scan_results[BW16_MAX_SCAN];
  int            bw16_scan_count = 0;

  // scan callback
  rtw_result_t scanResultHandler(rtw_scan_handler_result_t *scan_result)
  {
      if (scan_result->scan_complete == 0 && bw16_scan_count < BW16_MAX_SCAN)
      {
          auto *rec = &scan_result->ap_details;

          // Null-terminate SSID
          rec->SSID.val[rec->SSID.len] = '\0';

          // Populate our slot
          auto &slot = bw16_scan_results[bw16_scan_count++];
          slot.ssid     = String((char*)rec->SSID.val);
          slot.channel  = rec->channel;
          slot.rssi     = rec->signal_strength;
          memcpy(slot.bssid, rec->BSSID.octet, 6);

          // Format BSSID as XX:XX:XX:XX:XX:XX
          char buf[18];
          snprintf(buf, sizeof(buf),
                   "%02X:%02X:%02X:%02X:%02X:%02X",
                   rec->BSSID.octet[0], rec->BSSID.octet[1],
                   rec->BSSID.octet[2], rec->BSSID.octet[3],
                   rec->BSSID.octet[4], rec->BSSID.octet[5]);
          slot.bssid_str = String(buf);

          // Capture the security enum
          slot.security = rec->security;
      }
      return RTW_SUCCESS;
  }

  // Kick off a full scan, filling bw16_scan_results[]
  bool bw16_scan(int timeout = 10)
  {
      bw16_scan_count = 0;
      if(wifi_scan_networks(scanResultHandler, NULL) == RTW_SUCCESS)
      {
        delay(timeout * 1000);
        return true;
      }
      return false;
  }
#endif  // BOARD_BW16
*/