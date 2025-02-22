/* FlipperHTTP.cpp for flipper-http.ino and FlipperHTTPh.h
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over tthis->uart.
Created: 2024-09-30
Updated: 2025-02-21
*/

#include "FlipperHTTP.h"
#include "boards.h"

#ifdef BOARD_PICO_W
#include <LittleFS.h>
#elif BOARD_PICO_2W
#include <LittleFS.h>
#elif BOARD_VGM
#include <LittleFS.h>
#else
#include <SPIFFS.h>
#endif

// Clear serial buffer to avoid any residual data
void FlipperHTTP::clearSerialBuffer()
{
    this->uart.clear_buffer();
}

//  Connect to Wifi using the loaded SSID and Password
bool FlipperHTTP::connectToWifi()
{
    if (String(loadedSSID) == "" || String(loadedPassword) == "")
    {
        this->uart.println(F("[ERROR] WiFi SSID or Password is empty."));
        return false;
    }

    WiFi.disconnect(true); // Ensure WiFi is disconnected before reconnecting
    WiFi.begin(loadedSSID, loadedPassword);
#ifdef BOARD_ESP32_C3
    WiFi.setTxPower(WIFI_POWER_8_5dBm);
#endif

    int i = 0;
    while (!this->isConnectedToWifi() && i < 20)
    {
        delay(500);
        i++;
        this->uart.print(".");
    }
    this->uart.println(); // Move to next line after dots

    if (this->isConnectedToWifi())
    {
        this->uart.println(F("[SUCCESS] Successfully connected to Wifi."));
        configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // get UTC time via NTP
        return true;
    }
    else
    {
        this->uart.println(F("[ERROR] Failed to connect to Wifi."));
        return false;
    }
}

String FlipperHTTP::request(
    const char *method,
    String url,
    String payload,
    const char *headerKeys[],
    const char *headerValues[],
    int headerSize)
{
    HTTPClient http;
    String response = "";

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        if (payload == "")
        {
            payload = "{}";
        }

        int httpCode = http.sendRequest(method, payload);

        if (httpCode > 0)
        {
            response = http.getString();
            http.end();
            return response;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->uart.print(F("[ERROR] Request Failed, error: "));
                this->uart.println(F(http.errorToString(httpCode).c_str()));
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    for (int i = 0; i < headerSize; i++)
                    {
                        http.addHeader(headerKeys[i], headerValues[i]);
                    }
                    int newCode = http.sendRequest(method, payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca);
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca);
                        this->uart.println(F("[ERROR] Request Failed"));
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->uart.println(F("[ERROR] Unable to connect to the server."));
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

// Load WiFi settings from SPIFFS and attempt to connect
bool FlipperHTTP::loadWifiSettings()
{
#ifdef BOARD_PICO_W
    File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_PICO_2W
    File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_VGM
    File file = LittleFS.open(settingsFilePath, "r");
#else
    File file = SPIFFS.open(settingsFilePath, FILE_READ);
#endif
    if (!file)
    {
        return false;
    }

    // Read the entire file content
    String fileContent = file.readString();
    file.close();

    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, fileContent);

    if (error)
    {
        return false;
    }

    JsonArray wifiList = doc["wifi_list"].as<JsonArray>();
    for (JsonObject wifi : wifiList)
    {
        const char *ssid = wifi["ssid"];
        const char *password = wifi["password"];

        strlcpy(loadedSSID, ssid, sizeof(loadedSSID));
        strlcpy(loadedPassword, password, sizeof(loadedPassword));

        WiFi.begin(ssid, password);

        int attempts = 0;
        while (!this->isConnectedToWifi() && attempts < 4) // 2 seconds total, 500ms delay each
        {
            delay(500);
            attempts++;
        }

        if (this->isConnectedToWifi())
        {
            configTime(0, 0, "pool.ntp.org", "time.nist.gov"); // get UTC time via NTP
            return true;
        }
    }

    return false;
}

// Save WiFi settings to storage
bool FlipperHTTP::saveWifiSettings(String jsonData)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error)
    {
        this->uart.println(F("[ERROR] Failed to parse JSON data."));
        return false;
    }

    const char *newSSID = doc["ssid"];
    const char *newPassword = doc["password"];

    // Load existing settings if they exist
    JsonDocument existingDoc;
#ifdef BOARD_PICO_W
    File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_PICO_2W
    File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_VGM
    File file = LittleFS.open(settingsFilePath, "r");
#else
    File file = SPIFFS.open(settingsFilePath, FILE_READ);
#endif
    if (file)
    {
        deserializeJson(existingDoc, file);
        file.close();
    }

    // Check if SSID is already saved
    bool found = false;
    for (JsonObject wifi : existingDoc["wifi_list"].as<JsonArray>())
    {
        if (wifi["ssid"] == newSSID)
        {
            found = true;
            break;
        }
    }

    // Add new SSID and password if not found
    if (!found)
    {
        JsonArray wifiList = existingDoc["wifi_list"].to<JsonArray>();
        JsonObject newWifi = wifiList.createNestedObject();
        newWifi["ssid"] = newSSID;
        newWifi["password"] = newPassword;

// Save updated list to file
#ifdef BOARD_PICO_W
        file = LittleFS.open(settingsFilePath, "w");
#elif BOARD_PICO_2W
        file = LittleFS.open(settingsFilePath, "w");
#elif BOARD_VGM
        file = LittleFS.open(settingsFilePath, "w");
#else
        file = SPIFFS.open(settingsFilePath, FILE_WRITE);
#endif
        if (!file)
        {
            this->uart.println(F("[ERROR] Failed to open file for writing."));
            return false;
        }

        serializeJson(existingDoc, file);
        file.close();
    }

    this->uart.print(F("[SUCCESS] Settings saved."));
    return true;
}

// returns a string of all wifi networks
String FlipperHTTP::scanWifiNetworks()
{
    int n = WiFi.scanNetworks();
    String networks = "";
    for (int i = 0; i < n; ++i)
    {
        networks += WiFi.SSID(i);

        if (i < n - 1)
        {
            networks += ", ";
        }
    }
    return networks;
}

void FlipperHTTP::setup()
{
#ifdef BOARD_VGM
    this->uart.set_pins(0, 1);
#endif
    this->uart.begin(115200);
    this->uart.set_timeout(5000);
#ifdef BOARD_PICO_W
    if (!LittleFS.begin())
    {
        if (LittleFS.format())
        {
            if (!LittleFS.begin())
            {
                this->uart.println(F("Failed to re-mount LittleFS after formatting."));
                rp2040.reboot();
            }
        }
        else
        {
            this->uart.println(F("File system formatting failed."));
            rp2040.reboot();
        }
    }
#elif BOARD_PICO_2W
    if (!LittleFS.begin())
    {
        if (LittleFS.format())
        {
            if (!LittleFS.begin())
            {
                this->uart.println(F("Failed to re-mount LittleFS after formatting."));
                rp2040.reboot();
            }
        }
        else
        {
            this->uart.println(F("File system formatting failed."));
            rp2040.reboot();
        }
    }
#elif BOARD_VGM
    this->uart_2.set_pins(24, 21);
    this->uart_2.begin(115200);
    this->uart_2.set_timeout(5000);
    if (!LittleFS.begin())
    {
        if (LittleFS.format())
        {
            if (!LittleFS.begin())
            {
                this->uart.println(F("Failed to re-mount LittleFS after formatting."));
                rp2040.reboot();
            }
        }
        else
        {
            this->uart.println(F("File system formatting failed."));
            rp2040.reboot();
        }
    }
    this->uart_2.flush();
#else
    // Initialize SPIFFS
    if (!SPIFFS.begin(true))
    {
        this->uart.println(F("[ERROR] SPIFFS initialization failed."));
        ESP.restart();
    }
#endif
    this->useLED = true;
    this->led.start();
    this->loadWifiSettings();
    this->client.setCACert(this->root_ca);
    this->uart.flush();
}

bool FlipperHTTP::stream_bytes(const char *method, String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
{
    HTTPClient http;

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        if (payload == "")
        {
            payload = "{}";
        }

        int httpCode = http.sendRequest(method, payload);
        if (httpCode > 0)
        {
            if (strcmp(method, "GET") == 0)
            {
                this->uart.println(F("[GET/SUCCESS]"));
            }
            else
            {
                this->uart.println(F("[POST/SUCCESS]"));
            }
            int len = http.getSize(); // Get the response content length
            uint8_t buff[512] = {0};  // Buffer for reading data

            WiFiClient *stream = http.getStreamPtr();

// Check available heap memory before starting
#ifdef BOARD_PICO_W
            size_t freeHeap = rp2040.getFreeHeap();
#elif BOARD_PICO_2W
            size_t freeHeap = rp2040.getFreeHeap();
#elif BOARD_VGM
            size_t freeHeap = rp2040.getFreeHeap();
#else
            size_t freeHeap = ESP.getFreeHeap();
#endif
            const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
            if (freeHeap < minHeapThreshold)
            {
                this->uart.println(F("[ERROR] Not enough memory to start processing the response."));
                http.end();
                return false;
            }

            // Start timeout timer
            unsigned long timeoutStart = millis();
            const unsigned long timeoutInterval = 2000; // 2 seconds

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
                size_t size = stream->available();
                if (size)
                {
                    // Reset the timeout when new data comes in
                    timeoutStart = millis();

                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    this->uart.write(buff, c); // Write data to serial
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                else
                {
                    // Check if timeout has been reached
                    if (millis() - timeoutStart > timeoutInterval)
                    {
                        break;
                    }
                }
                delay(1); // Yield control to the system
            }

#ifdef BOARD_PICO_W
            freeHeap = rp2040.getFreeHeap();
#elif BOARD_PICO_2W
            freeHeap = rp2040.getFreeHeap();
#elif BOARD_VGM
            freeHeap = rp2040.getFreeHeap();
#else
            freeHeap = ESP.getFreeHeap();
#endif
            if (freeHeap < minHeapThreshold)
            {
                this->uart.println(F("[ERROR] Not enough memory to continue processing the response."));
                http.end();
                return false;
            }

            http.end();
            // Flush the serial buffer to ensure all data is sent
            this->uart.flush();
            this->uart.println();
            if (strcmp(method, "GET") == 0)
            {
                this->uart.println("[GET/END]");
            }
            else
            {
                this->uart.println("[POST/END]");
            }
            return true;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->uart.print(F("[ERROR] Request Failed, error: "));
                this->uart.println(F(http.errorToString(httpCode).c_str()));
            }
            else // certification failed?
            {
                // Send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    for (int i = 0; i < headerSize; i++)
                    {
                        http.addHeader(headerKeys[i], headerValues[i]);
                    }
                    int newCode = http.sendRequest(method, payload);
                    if (newCode > 0)
                    {
                        if (strcmp(method, "GET") == 0)
                        {
                            this->uart.println(F("[GET/SUCCESS]"));
                        }
                        else
                        {
                            this->uart.println(F("[POST/SUCCESS]"));
                        }
                        int len = http.getSize(); // Get the response content length
                        uint8_t buff[512] = {0};  // Buffer for reading data

                        WiFiClient *stream = http.getStreamPtr();

// Check available heap memory before starting
#ifdef BOARD_PICO_W
                        size_t freeHeap = rp2040.getFreeHeap();
#elif BOARD_PICO_2W
                        size_t freeHeap = rp2040.getFreeHeap();
#elif BOARD_VGM
                        size_t freeHeap = rp2040.getFreeHeap();
#else
                        size_t freeHeap = ESP.getFreeHeap();
#endif
                        if (freeHeap < 1024)
                        {
                            this->uart.println(F("[ERROR] Not enough memory to start processing the response."));
                            http.end();
                            this->client.setCACert(this->root_ca);
                            return false;
                        }

                        // Start timeout timer
                        unsigned long timeoutStart = millis();
                        const unsigned long timeoutInterval = 2000; // 2 seconds

                        // Stream data while connected and available
                        while (http.connected() && (len > 0 || len == -1))
                        {
                            size_t size = stream->available();
                            if (size)
                            {
                                // Reset the timeout when new data arrives
                                timeoutStart = millis();

                                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                                this->uart.write(buff, c); // Write data to serial
                                if (len > 0)
                                {
                                    len -= c;
                                }
                            }
                            else
                            {
                                // Check if timeout has been reached
                                if (millis() - timeoutStart > timeoutInterval)
                                {
                                    break;
                                }
                            }
                            delay(1); // Yield control to the system
                        }

#ifdef BOARD_PICO_W
                        freeHeap = rp2040.getFreeHeap();
#elif BOARD_PICO_2W
                        freeHeap = rp2040.getFreeHeap();
#elif BOARD_VGM
                        freeHeap = rp2040.getFreeHeap();
#else
                        freeHeap = ESP.getFreeHeap();
#endif
                        if (freeHeap < 1024)
                        {
                            this->uart.println(F("[ERROR] Not enough memory to continue processing the response."));
                            http.end();
                            this->client.setCACert(this->root_ca);
                            return false;
                        }

                        http.end();
                        // Flush the serial buffer to ensure all data is sent
                        this->uart.flush();
                        this->uart.println();
                        if (strcmp(method, "GET") == 0)
                        {
                            this->uart.println("[GET/END]");
                        }
                        else
                        {
                            this->uart.println("[POST/END]");
                        }
                        this->client.setCACert(this->root_ca);
                        return true;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca);
                        this->uart.printf("[ERROR] Request failed with error: %s\n", http.errorToString(httpCode).c_str());
                    }
                }
                this->client.setCACert(this->root_ca);
            }
        }
        http.end();
    }
    else
    {
        this->uart.println(F("[ERROR] Unable to connect to the server."));
    }
    return false;
}

bool FlipperHTTP::read_serial_settings(String receivedData, bool connectAfterSave)
{
    JsonDocument doc;
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error)
    {
        this->uart.print(F("[ERROR] Failed to parse JSON: "));
        this->uart.println(F(error.c_str()));
        return false;
    }

    // Extract values from JSON
    if (doc.containsKey("ssid") && doc.containsKey("password"))
    {
        strlcpy(loadedSSID, doc["ssid"], sizeof(loadedSSID));             // save ssid
        strlcpy(loadedPassword, doc["password"], sizeof(loadedPassword)); // save password
    }
    else
    {
        this->uart.println(F("[ERROR] JSON does not contain ssid and password."));
        return false;
    }

    // Save to storage
    if (!this->saveWifiSettings(receivedData))
    {
        this->uart.println(F("[ERROR] Failed to save settings to file."));
        return false;
    }

    // Attempt to reconnect with new settings
    if (connectAfterSave && this->connectToWifi())
    {
        this->uart.println(F("[SUCCESS] Connected to the new Wifi network."));
    }

    return true;
}

// Upload bytes to server
bool FlipperHTTP::upload_bytes(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
{
    HTTPClient http;

    // set headers
    http.collectHeaders(headerKeys, headerSize);

    // begin connection
    if (http.begin(this->client, url))
    {
        // add headers
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        // send the request
        int httpCode = http.POST(payload);
        if (httpCode > 0)
        {
            this->uart.println(F("[POST/SUCCESS]"));

            WiFiClient *stream = http.getStreamPtr();

            // send incoming serial data to the server
            if (this->uart.available() > 0)
            {
                uint8_t buffer[128];
                size_t len;
                while ((len = this->uart.readBytes(buffer, sizeof(buffer))) > 0)
                {
                    stream->write(buffer, len);
                }
                this->uart.flush();
            }

            // end the request
            http.end();
            // Flush the serial buffer to ensure all data is sent
            this->uart.flush();
            this->uart.println();
            this->uart.println(F("[POST/END]"));
            return true;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->uart.print(F("[ERROR] POST Request Failed, error: "));
                this->uart.println(F(http.errorToString(httpCode).c_str()));
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    for (int i = 0; i < headerSize; i++)
                    {
                        http.addHeader(headerKeys[i], headerValues[i]);
                    }
                    int newCode = http.POST(payload);
                    if (newCode > 0)
                    {
                        this->uart.println(F("[POST/SUCCESS]"));

                        WiFiClient *stream = http.getStreamPtr();

                        // send incoming serial data to the server
                        while (this->uart.available() > 0)
                        {
                            stream->write(this->uart.read());
                        }

                        // end the request
                        http.end();
                        // Flush the serial buffer to ensure all data is sent
                        this->uart.flush();
                        this->uart.println();
                        this->uart.println(F("[POST/END]"));
                        this->client.setCACert(this->root_ca);
                        return true;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca);
                        this->uart.println(F("[ERROR] POST Request Failed"));
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->uart.println(F("[ERROR] Unable to connect to the server."));
    }
    return false;
}
// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
#ifdef BOARD_VGM
    // Check if there's incoming serial data
    if (this->uart.available() > 0)
    {
        this->led.on();

        // Read the incoming serial data until newline
        String _data = this->uart.read_serial_line();

        // send to ESP32
        this->uart_2.println(_data);

        // Wait for response from ESP32
        String _response = this->uart_2.read_serial_line();

        // Send response back to Flipper
        this->uart.println(_response);

        this->led.off();
    }
    else if (this->uart_2.available() > 0)
    {
        this->led.on();

        // Read the incoming serial data until newline
        String _data = this->uart_2.read_serial_line();

        // send to Flipper
        this->uart.println(_data);

        this->led.off();
    }
#else
    // Check if there's incoming serial data
    if (this->uart.available() > 0)
    {
        // Read the incoming serial data until newline
        String _data = this->uart.read_serial_line();

        if (_data.length() == 0)
        {
            // No complete command received
            return;
        }

        this->led.on();

        // print the available commands
        if (_data.startsWith("[LIST]"))
        {
            this->uart.println(F("[LIST],[PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS]"));
        }
        // handle [LED/ON] command
        else if (_data.startsWith("[LED/ON]"))
        {
            this->useLED = true;
        }
        // handle [LED/OFF] command
        else if (_data.startsWith("[LED/OFF]"))
        {
            this->useLED = false;
        }
        // handle [IP/ADDRESS] command (local IP)
        else if (_data.startsWith("[IP/ADDRESS]"))
        {
            this->uart.println(this->getIPAddress());
        }
        // handle [WIFI/IP] command ip of connected wifi
        else if (_data.startsWith("[WIFI/IP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }
            // Get Request
            String jsonData = this->request("GET", "https://httpbin.org/get");
            if (jsonData == "")
            {
                this->uart.println(F("[ERROR] GET request failed or returned empty data."));
                return;
            }
            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);
            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }
            if (!doc.containsKey("origin"))
            {
                this->uart.println(F("[ERROR] JSON does not contain origin."));
                this->led.off();
                return;
            }
            this->uart.println(doc["origin"].as<String>());
        }
        // Ping/Pong to see if board/flipper is connected
        else if (_data.startsWith("[PING]"))
        {
            this->uart.println("[PONG]");
        }
        // Handle [REBOOT] command
        else if (_data.startsWith("[REBOOT]"))
        {
            this->useLED = true;
#ifdef BOARD_PICO_W
            rp2040.reboot();
#elif BOARD_PICO_2W
            rp2040.reboot();
#elif BOARD_VGM
            rp2040.reboot();
#else
            ESP.restart();
#endif
        }
        // scan for wifi networks
        else if (_data.startsWith("[WIFI/SCAN]"))
        {
            this->uart.println(this->scanWifiNetworks());
            this->uart.flush();
        }
        // Handle Wifi list command
        else if (_data.startsWith("[WIFI/LIST]"))
        {
#ifdef BOARD_PICO_W
            File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_PICO_2W
            File file = LittleFS.open(settingsFilePath, "r");
#elif BOARD_VGM
            File file = LittleFS.open(settingsFilePath, "r");
#else
            File file = SPIFFS.open(settingsFilePath, FILE_READ);
#endif
            if (!file)
            {
                this->uart.println(F("[ERROR] Failed to open file for reading."));
                return;
            }

            // Read the entire file content
            String fileContent = file.readString();
            file.close();

            this->uart.println(fileContent);
            this->uart.flush();
        }
        // Handle [WIFI/SAVE] command
        else if (_data.startsWith("[WIFI/SAVE]"))
        {
            // Extract JSON data by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/SAVE]"));
            jsonData.trim(); // Remove any leading/trailing whitespace

            // Parse and save the settings
            if (this->read_serial_settings(jsonData, true))
            {
                this->uart.println(F("[SUCCESS] Wifi settings saved."));
            }
            else
            {
                this->uart.println(F("[ERROR] Failed to save Wifi settings."));
            }
        }
        // Handle [WIFI/CONNECT] command
        else if (_data == "[WIFI/CONNECT]")
        {
            // Check if WiFi is already connected
            if (!this->isConnectedToWifi())
            {
                // Attempt to connect to Wifi
                if (this->connectToWifi())
                {
                    this->uart.println(F("[SUCCESS] Connected to Wifi."));
                }
                else
                {
                    this->uart.println(F("[ERROR] Failed to connect to Wifi."));
                }
            }
            else
            {
                this->uart.println(F("[INFO] Already connected to WiFi."));
            }
        }
        // Handle [WIFI/DISCONNECT] command
        else if (_data == "[WIFI/DISCONNECT]")
        {
            WiFi.disconnect(true);
            this->uart.println(F("[DISCONNECTED] WiFi has been disconnected."));
        }
        // Handle [GET] command
        else if (_data.startsWith("[GET]"))
        {

            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to WiFi. Failed to reconnect."));
                this->led.off();
                return;
            }
            // Extract URL by removing the command part
            String url = _data.substring(strlen("[GET]"));
            url.trim();

            // GET request
            String getData = this->request("GET", url);
            if (getData != "")
            {
                this->uart.println(F("[GET/SUCCESS] GET request successful."));
                this->uart.println(getData);
                this->uart.flush();
                this->uart.println();
                this->uart.println(F("[GET/END]"));
            }
            else
            {
                this->uart.println(F("[ERROR] GET request failed or returned empty data."));
            }
        }
        // Handle [GET/HTTP] command
        else if (_data.startsWith("[GET/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            String url = doc["url"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // GET request
            String getData = this->request("GET", url, "", headerKeys, headerValues, headerSize);
            if (getData != "")
            {
                this->uart.println(F("[GET/SUCCESS] GET request successful."));
                this->uart.println(getData);
                this->uart.flush();
                this->uart.println();
                this->uart.println(F("[GET/END]"));
            }
            else
            {
                this->uart.println(F("[ERROR] GET request failed or returned empty data."));
            }
        }
        // Handle [POST/HTTP] command
        else if (_data.startsWith("[POST/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // POST request
            String postData = this->request("POST", url, payload, headerKeys, headerValues, headerSize);
            if (postData != "")
            {
                this->uart.println(F("[POST/SUCCESS] POST request successful."));
                this->uart.println(postData);
                this->uart.flush();
                this->uart.println();
                this->uart.println(F("[POST/END]"));
            }
            else
            {
                this->uart.println(F("[ERROR] POST request failed or returned empty data."));
            }
        }
        // Handle [PUT/HTTP] command
        else if (_data.startsWith("[PUT/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PUT/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print("[ERROR] Failed to parse JSON.");
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // PUT request
            String putData = this->request("PUT", url, payload, headerKeys, headerValues, headerSize);
            if (putData != "")
            {
                this->uart.println(F("[PUT/SUCCESS] PUT request successful."));
                this->uart.println(putData);
                this->uart.flush();
                this->uart.println();
                this->uart.println(F("[PUT/END]"));
            }
            else
            {
                this->uart.println(F("[ERROR] PUT request failed or returned empty data."));
            }
        }
        // Handle [DELETE/HTTP] command
        else if (_data.startsWith("[DELETE/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[DELETE/HTTP]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // DELETE request
            String deleteData = this->request("DELETE", url, payload, headerKeys, headerValues, headerSize);
            if (deleteData != "")
            {
                this->uart.println(F("[DELETE/SUCCESS] DELETE request successful."));
                this->uart.println(deleteData);
                this->uart.flush();
                this->uart.println();
                this->uart.println(F("[DELETE/END]"));
            }
            else
            {
                this->uart.println(F("[ERROR] DELETE request failed or returned empty data."));
            }
        }

        // Handle [GET/BYTES]
        else if (_data.startsWith("[GET/BYTES]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/BYTES]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            String url = doc["url"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // GET request
            if (!this->stream_bytes("GET", url, "", headerKeys, headerValues, headerSize))
            {
                this->uart.println(F("[ERROR] GET request failed or returned empty data."));
            }
        }
        // handle [POST/BYTES]
        else if (_data.startsWith("[POST/BYTES]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->uart.println(F("[ERROR] Not connected to Wifi. Failed to reconnect."));
                this->led.off();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/BYTES]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url or payload."));
                this->led.off();
                return;
            }
            String url = doc["url"];
            String payload = doc["payload"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // POST request
            if (!this->stream_bytes("POST", url, payload, headerKeys, headerValues, headerSize))
            {
                this->uart.println(F("[ERROR] POST request failed or returned empty data."));
            }
        }
        // Handle [PARSE] command
        // the user will append the key to read from the json
        // example: [PARSE]{"key":"name","json":{"name":"John Doe"}}
        else if (_data.startsWith("[PARSE]"))
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PARSE]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("key") || !doc.containsKey("json"))
            {
                this->uart.println(F("[ERROR] JSON does not contain key or json."));
                this->led.off();
                return;
            }
            String key = doc["key"];
            JsonObject json = doc["json"];

            if (json.containsKey(key))
            {
                this->uart.println(json[key].as<String>());
            }
            else
            {
                this->uart.println(F("[ERROR] Key not found in JSON."));
            }
        }
        // Handle [PARSE/ARRAY] command
        // the user will append the key to read and the index of the array to get it's key from the json
        // example: [PARSE/ARRAY]{"key":"name","index":"1","json":{"name":["John Doe","Jane Doe"]}}
        // this would return Jane Doe
        // and in this example it would return {"flavor": "red"}:
        // example: [PARSE/ARRAY]{"key":"flavor","index":"1","json":{"name":[{"flavor": "blue"},{"flavor": "red"}]}}
        else if (_data.startsWith("[PARSE/ARRAY]"))
        {
            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PARSE/ARRAY]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("key") || !doc.containsKey("index") || !doc.containsKey("json"))
            {
                this->uart.println(F("[ERROR] JSON does not contain key, index, or json."));
                this->led.off();
                return;
            }
            String key = doc["key"];
            int index = doc["index"];
            JsonArray json = doc["json"];

            if (json[index].containsKey(key))
            {
                this->uart.println(json[index][key].as<String>());
            }
            else
            {
                this->uart.println(F("[ERROR] Key not found in JSON."));
            }
        }
        // websocket
        else if (_data.startsWith("[SOCKET/START]"))
        {
            // extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[SOCKET/START]"));
            jsonData.trim();

            JsonDocument doc;
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->uart.print(F("[ERROR] Failed to parse JSON."));
                this->led.off();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url"))
            {
                this->uart.println(F("[ERROR] JSON does not contain url."));
                this->led.off();
                return;
            }
            const char *url = doc["url"];
            if (!doc.containsKey("port"))
            {
                this->uart.println(F("[ERROR] JSON does not contain port."));
                this->led.off();
                return;
            }
            int port = doc["port"];

            // Extract headers if available
            const char *headerKeys[10];
            const char *headerValues[10];
            int headerSize = 0;

            if (doc.containsKey("headers"))
            {
                JsonObject headers = doc["headers"];
                for (JsonPair header : headers)
                {
                    headerKeys[headerSize] = header.key().c_str();
                    headerValues[headerSize] = header.value();
                    headerSize++;
                }
            }

            // start the websocket
            WebSocketClient ws = WebSocketClient(this->client, url, port);

            // Begin the WebSocket connection (performs the handshake)
            ws.begin();

            // Check if a message is available from the server:
            if (ws.parseMessage() > 0)
            {
                // Read the message from the server
                String message = ws.readString();
                this->uart.println(message);
            }

            // wait for incoming serial/client data, and send back-n-forth
            String uartMessage = "";
            String wsMessage = "";
            while (ws.connected() && !uartMessage.startsWith("[SOCKET/STOP]"))
            {
                // Check if there's incoming serial data
                if (this->uart.available() > 0)
                {
                    // Read the incoming serial data until newline
                    uartMessage = this->uart.read_serial_line();
                    ws.beginMessage(TYPE_TEXT);
                    ws.print(uartMessage);
                    ws.endMessage();
                }

                // Check if there's incoming websocket data
                if (ws.parseMessage() > 0)
                {
                    // Read the message from the server
                    wsMessage = ws.readString();
                    this->uart.println(wsMessage);
                }
            }
            // Close the WebSocket connection
            ws.stop();
        }

        this->led.off();
    }
#endif
}
