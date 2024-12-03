/* FlipperHTTPPico.h for flipper-http-pico.ino (for the Raspberry Pi Pico W)
Author: JBlanked
Github: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: Raspberry Pi Pico W
Created: 2024-10-24
Updated: 2024-12-02

Change Log:
- 2024-10-24: Initial commit
- 2024-10-25: Updated the readSerialLine method to use readStringUntil instead of reading char by char, and added Auto connect
  - Reading char by char worked on the dev board but was missing chars on the Pico
- 2024-10-26: Updated the saveWifiSettings and loadWifiSettings methods to save and load a list of wifi networks, and added [WIFI/LIST] command
- 2024-11-09: Added SSL certificate from https://letsencrypt.org/certificates/
- 2024-11-15: Updated to check heap during [GET/BYTES] and [POST/BYTES] to avoid memory issues
- 2024-12-02: Restructured the code
*/
#include "FlipperHTTPPico.h"

// Clear serial buffer to avoid any residual data
void FlipperHTTP::clearSerialBuffer()
{
    while (this->SerialPico->available() > 0)
    {
        this->SerialPico->read();
    }
}

//  Connect to Wifi using the loaded SSID and Password
bool FlipperHTTP::connectToWifi()
{
    if (String(loadedSSID) == "" || String(loadedPassword) == "")
    {
        this->SerialPico->println("[ERROR] WiFi SSID or Password is empty.");
        return false;
    }

    WiFi.disconnect(true); // Ensure WiFi is disconnected before reconnecting
    WiFi.begin(loadedSSID, loadedPassword);

    int i = 0;
    while (!this->isConnectedToWifi() && i < 20)
    {
        delay(500);
        i++;
        this->SerialPico->print(".");
    }
    this->SerialPico->println(); // Move to next line after dots

    if (this->isConnectedToWifi())
    {
        this->SerialPico->println("[SUCCESS] Successfully connected to Wifi.");
        return true;
    }
    else
    {
        this->SerialPico->println("[ERROR] Failed to connect to Wifi.");
        return false;
    }
}

String FlipperHTTP::delete_request(String url, String payload)
{

    HTTPClient http;
    String response = "";

    if (http.begin(this->client, url))
    {
        int httpCode = http.sendRequest("DELETE", payload);

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
                this->SerialPico->print("[ERROR] DELETE Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    int newCode = http.sendRequest("DELETE", payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] DELETE Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::delete_request(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
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

        int httpCode = http.sendRequest("DELETE", payload);

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
                this->SerialPico->print("[ERROR] DELETE Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                    int newCode = http.sendRequest("DELETE", payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] DELETE Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::get(String url)
{

    HTTPClient http;
    String payload = "";

    if (http.begin(this->client, url))
    {
        int httpCode = http.GET();

        if (httpCode > 0)
        {
            payload = http.getString();
            http.end();
            return payload;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->SerialPico->print("[ERROR] GET Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    int newCode = http.GET();
                    if (newCode > 0)
                    {
                        payload = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return payload;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] GET Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return payload;
}

String FlipperHTTP::get(String url, const char *headerKeys[], const char *headerValues[], int headerSize)
{

    HTTPClient http;
    String payload = "";

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {

        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.GET();

        if (httpCode > 0)
        {
            payload = http.getString();
            http.end();
            return payload;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->SerialPico->print("[ERROR] GET Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                    int newCode = http.GET();
                    if (newCode > 0)
                    {
                        payload = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return payload;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] GET Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return payload;
}

bool FlipperHTTP::get_bytes_to_file(String url, const char *headerKeys[], const char *headerValues[], int headerSize)
{

    HTTPClient http;

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.GET();
        if (httpCode > 0)
        {
            this->SerialPico->println("[GET/SUCCESS]");

            int len = http.getSize();
            uint8_t buff[512] = {0};

            WiFiClient *stream = http.getStreamPtr();

            // Check available heap memory before starting
            size_t freeHeap = rp2040.getFreeHeap();
            const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
            if (freeHeap < minHeapThreshold)
            {
                this->SerialPico->println("[ERROR] Not enough memory to start processing the response.");
                http.end();
                return false;
            }

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
                size_t size = stream->available();
                if (size)
                {
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    this->SerialPico->write(buff, c); // Write data to serial
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                delay(1); // Yield control to the system
            }

            freeHeap = rp2040.getFreeHeap();
            if (freeHeap < minHeapThreshold)
            {
                this->SerialPico->println("[ERROR] Not enough memory to continue processing the response.");
                http.end();
                return false;
            }

            // Flush the serial buffer to ensure all data is sent
            http.end();
            this->SerialPico->flush();
            this->SerialPico->println();
            this->SerialPico->println("[GET/END]");

            return true;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->SerialPico->print("[ERROR] GET Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                    int newCode = http.GET();
                    if (newCode > 0)
                    {
                        this->SerialPico->println("[GET/SUCCESS]");
                        int len = http.getSize();
                        uint8_t buff[512] = {0};

                        WiFiClient *stream = http.getStreamPtr();

                        // Check available heap memory before starting
                        size_t freeHeap = rp2040.getFreeHeap();
                        const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
                        if (freeHeap < minHeapThreshold)
                        {
                            this->SerialPico->println("[ERROR] Not enough memory to start processing the response.");
                            http.end();
                            return false;
                        }

                        // Stream data while connected and available
                        while (http.connected() && (len > 0 || len == -1))
                        {
                            size_t size = stream->available();
                            if (size)
                            {
                                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                                this->SerialPico->write(buff, c); // Write data to serial
                                if (len > 0)
                                {
                                    len -= c;
                                }
                            }
                            delay(1); // Yield control to the system
                        }

                        freeHeap = rp2040.getFreeHeap();
                        if (freeHeap < minHeapThreshold)
                        {
                            this->SerialPico->println("[ERROR] Not enough memory to continue processing the response.");
                            http.end();
                            return false;
                        }

                        // Flush the serial buffer to ensure all data is sent
                        http.end();
                        this->SerialPico->flush();
                        this->SerialPico->println();
                        this->SerialPico->println("[GET/END]");
                        this->client.setCACert(this->root_ca); // reset to secure
                        return true;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->printf("[ERROR] GET request failed with error: %s\n", http.errorToString(newCode).c_str());
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }
    return false;
}

// Function to flash the LED
void FlipperHTTP::ledAction(int timeout)
{
    digitalWrite(PICO_LED, ON);
    delay(timeout);
    digitalWrite(PICO_LED, OFF);
    delay(timeout);
}

// Display LED sequence when Wifi Board is first connected to the Flipper
void FlipperHTTP::ledStart()
{
    // Initialize LED pin
    pinMode(PICO_LED, OUTPUT);

    ledAction();
    ledAction();
    ledAction();
}

// Starting LED (Green only)
void FlipperHTTP::ledStatus()
{
    if (this->useLED)
    {
        digitalWrite(PICO_LED, ON);
    }
}

bool FlipperHTTP::loadWifiSettings()
{
    File file = LittleFS.open(settingsFilePath, "r");
    if (!file)
    {
        return false;
    }

    String fileContent = file.readString();
    file.close();

    DynamicJsonDocument doc(2048);
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
            this->SerialPico->print(".");
        }

        if (this->isConnectedToWifi())
        {
            return true;
        }
    }
    return false;
}

String FlipperHTTP::post(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
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

        int httpCode = http.POST(payload);

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
                this->SerialPico->print("[ERROR] POST Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] POST Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::post(String url, String payload)
{

    HTTPClient http;
    String response = "";

    if (http.begin(this->client, url))
    {

        int httpCode = http.POST(payload);

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
                this->SerialPico->print("[ERROR] POST Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    int newCode = http.POST(payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] POST Request Failed");
                    }
                }
            }
        }
        http.end();
    }

    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

bool FlipperHTTP::post_bytes_to_file(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
{

    HTTPClient http;

    http.collectHeaders(headerKeys, headerSize);

    if (http.begin(this->client, url))
    {
        for (int i = 0; i < headerSize; i++)
        {
            http.addHeader(headerKeys[i], headerValues[i]);
        }

        int httpCode = http.POST(payload);
        if (httpCode > 0)
        {
            this->SerialPico->println("[POST/SUCCESS]");

            int len = http.getSize(); // Get the response content length
            uint8_t buff[512] = {0};  // Buffer for reading data

            WiFiClient *stream = http.getStreamPtr();

            // Check available heap memory before starting
            size_t freeHeap = rp2040.getFreeHeap();
            const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
            if (freeHeap < minHeapThreshold)
            {
                this->SerialPico->println("[ERROR] Not enough memory to start processing the response.");
                http.end();
                return false;
            }

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
                size_t size = stream->available();
                if (size)
                {
                    int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                    this->SerialPico->write(buff, c); // Write data to serial
                    if (len > 0)
                    {
                        len -= c;
                    }
                }
                delay(1); // Yield control to the system
            }

            freeHeap = rp2040.getFreeHeap();
            if (freeHeap < minHeapThreshold)
            {
                this->SerialPico->println("[ERROR] Not enough memory to continue processing the response.");
                http.end();
                return false;
            }

            http.end();
            // Flush the serial buffer to ensure all data is sent
            this->SerialPico->flush();
            this->SerialPico->println();
            this->SerialPico->println("[POST/END]");

            return true;
        }
        else
        {
            if (httpCode != -1) // HTTPC_ERROR_CONNECTION_FAILED
            {
                this->SerialPico->print("[ERROR] POST Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                        this->SerialPico->println("[POST/SUCCESS]");
                        int len = http.getSize(); // Get the response content length
                        uint8_t buff[512] = {0};  // Buffer for reading data

                        WiFiClient *stream = http.getStreamPtr();

                        // Check available heap memory before starting
                        size_t freeHeap = rp2040.getFreeHeap();
                        const size_t minHeapThreshold = 1024; // Minimum heap space to avoid overflow
                        if (freeHeap < minHeapThreshold)
                        {
                            this->SerialPico->println("[ERROR] Not enough memory to start processing the response.");
                            http.end();
                            return false;
                        }

                        // Stream data while connected and available
                        while (http.connected() && (len > 0 || len == -1))
                        {
                            size_t size = stream->available();
                            if (size)
                            {
                                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                                this->SerialPico->write(buff, c); // Write data to serial
                                if (len > 0)
                                {
                                    len -= c;
                                }
                            }
                            delay(1); // Yield control to the system
                        }

                        freeHeap = rp2040.getFreeHeap();
                        if (freeHeap < minHeapThreshold)
                        {
                            this->SerialPico->println("[ERROR] Not enough memory to continue processing the response.");
                            http.end();
                            return false;
                        }

                        http.end();
                        // Flush the serial buffer to ensure all data is sent
                        this->SerialPico->flush();
                        this->SerialPico->println();
                        this->SerialPico->println("[POST/END]");
                        this->client.setCACert(this->root_ca); // reset to secure
                        return true;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->printf("[ERROR] POST request failed with error: %s\n", http.errorToString(newCode).c_str());
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }
    return false;
}

String FlipperHTTP::put(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize)
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

        int httpCode = http.PUT(payload);

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
                this->SerialPico->print("[ERROR] PUT Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
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
                    int newCode = http.PUT(payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] PUT Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

String FlipperHTTP::put(String url, String payload)
{

    HTTPClient http;
    String response = "";

    if (http.begin(this->client, url))
    {
        int httpCode = http.PUT(payload);

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
                this->SerialPico->print("[ERROR] PUT Request Failed, error: ");
                this->SerialPico->println(http.errorToString(httpCode).c_str());
            }
            else // certification failed?
            {
                // send request without SSL
                http.end();
                this->client.setInsecure();
                if (http.begin(this->client, url))
                {
                    int newCode = http.PUT(payload);
                    if (newCode > 0)
                    {
                        response = http.getString();
                        http.end();
                        this->client.setCACert(this->root_ca); // reset to secure
                        return response;
                    }
                    else
                    {
                        this->client.setCACert(this->root_ca); // reset to secure
                        this->SerialPico->println("[ERROR] PUT Request Failed");
                    }
                }
            }
        }
        http.end();
    }
    else
    {
        this->SerialPico->println("[ERROR] Unable to connect to the server.");
    }

    // Clear serial buffer to avoid any residual data
    this->clearSerialBuffer();

    return response;
}

// Save WiFi settings to LittleFS
bool FlipperHTTP::saveWifiSettings(String jsonData)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, jsonData);

    if (error)
    {
        this->SerialPico->println("[ERROR] Failed to parse JSON data.");
        return false;
    }

    const char *newSSID = doc["ssid"];
    const char *newPassword = doc["password"];

    // Load existing settings if they exist
    DynamicJsonDocument existingDoc(2048);
    File file = LittleFS.open(settingsFilePath, "r");
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
        file = LittleFS.open(settingsFilePath, "w");
        if (!file)
        {
            this->SerialPico->println("[ERROR] Failed to open file for writing.");
            return false;
        }

        serializeJson(existingDoc, file);
        file.close();
    }

    this->SerialPico->println("[SUCCESS] Settings saved to LittleFS.");
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
    this->SerialPico = new SerialPIO(0, 1);
    this->SerialPico->begin(BAUD_RATE);

    if (!LittleFS.begin())
    {
        if (LittleFS.format())
        {
            if (!LittleFS.begin())
            {
                this->SerialPico->println("Failed to re-mount LittleFS after formatting.");
                rp2040.reboot();
            }
        }
        else
        {
            this->SerialPico->println("File system formatting failed.");
            rp2040.reboot();
        }
    }
    this->useLED = true;
    this->ledStart();
    this->loadWifiSettings();
    this->client.setCACert(this->root_ca);
    this->SerialPico->flush();
}

String FlipperHTTP::readSerialLine()
{
    String receivedData = "";

    receivedData = this->SerialPico->readStringUntil('\n');

    receivedData.trim(); // Remove any leading/trailing whitespace

    return receivedData;
}

bool FlipperHTTP::readSerialSettings(String receivedData, bool connectAfterSave)
{
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, receivedData);

    if (error)
    {
        this->SerialPico->print("[ERROR] Failed to parse JSON: ");
        this->SerialPico->println(error.c_str());
        return false;
    }

    // Extract values from JSON
    if (doc.containsKey("ssid") && doc.containsKey("password"))
    {
        strlcpy(loadedSSID, doc["ssid"], sizeof(loadedSSID));
        strlcpy(loadedPassword, doc["password"], sizeof(loadedPassword));
    }
    else
    {
        this->SerialPico->println("[ERROR] JSON does not contain ssid and password.");
        return false;
    }

    // Save to SPIFFS
    if (!this->saveWifiSettings(receivedData))
    {
        this->SerialPico->println("[ERROR] Failed to save settings to file.");
        return false;
    }

    // Attempt to reconnect with new settings
    if (connectAfterSave && this->connectToWifi())
    {
        this->SerialPico->println("[SUCCESS] Connected to the new Wifi network.");
    }

    return true;
}

// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
    // Check if there's incoming serial data
    if (this->SerialPico->available() > 0)
    {
        // Read the incoming serial data until newline
        String _data = this->readSerialLine();

        this->ledStatus();

        // print the available commands
        if (_data.startsWith("[LIST]"))
        {
            this->SerialPico->println("[LIST],[PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS]");
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
            this->SerialPico->println(this->getIPAddress());
        }
        // handle [WIFI/IP] command ip of connected wifi
        else if (_data.startsWith("[WIFI/IP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }
            // Get Request
            String jsonData = this->get("https://httpbin.org/get");
            if (jsonData == "")
            {
                this->SerialPico->println("[ERROR] GET request failed or returned empty data.");
                return;
            }
            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);
            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }
            if (!doc.containsKey("origin"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain origin.");
                this->ledOff();
                return;
            }
            this->SerialPico->println(doc["origin"].as<String>());
        }
        // Ping/Pong to see if board/flipper is connected
        else if (_data.startsWith("[PING]"))
        {
            this->SerialPico->println("[PONG]");
        }
        // Handle [REBOOT] command
        else if (_data.startsWith("[REBOOT]"))
        {
            this->useLED = true;
            rp2040.reboot();
        }
        // scan for wifi networks
        else if (_data.startsWith("[WIFI/SCAN]"))
        {
            this->SerialPico->println(this->scanWifiNetworks());
            this->SerialPico->flush();
        }
        // Handle Wifi list command
        else if (_data.startsWith("[WIFI/LIST]"))
        {
            File file = LittleFS.open(settingsFilePath, "r");
            if (!file)
            {
                this->SerialPico->println("[ERROR] Failed to open file for reading.");
                return;
            }

            String fileContent = file.readString();
            file.close();

            this->SerialPico->println(fileContent);
            this->SerialPico->flush();
        }
        // Handle [WIFI/SAVE] command
        else if (_data.startsWith("[WIFI/SAVE]"))
        {
            // Extract JSON data by removing the command part
            String jsonData = _data.substring(strlen("[WIFI/SAVE]"));
            jsonData.trim(); // Remove any leading/trailing whitespace

            // Parse and save the settings
            if (this->readSerialSettings(jsonData, true))
            {
                this->SerialPico->println("[SUCCESS] Wifi settings saved.");
            }
            else
            {
                this->SerialPico->println("[ERROR] Failed to save Wifi settings.");
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
                    this->SerialPico->println("[SUCCESS] Connected to Wifi.");
                }
                else
                {
                    this->SerialPico->println("[ERROR] Failed to connect to Wifi.");
                }
            }
            else
            {
                this->SerialPico->println("[INFO] Already connected to Wifi.");
            }
        }
        // Handle [WIFI/DISCONNECT] command
        else if (_data == "[WIFI/DISCONNECT]")
        {
            WiFi.disconnect(true);
            this->SerialPico->println("[DISCONNECTED] Wifi has been disconnected.");
        }
        // Handle [GET] command
        else if (_data.startsWith("[GET]"))
        {

            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }
            // Extract URL by removing the command part
            String url = _data.substring(strlen("[GET]"));
            url.trim();

            // GET request
            String getData = this->get(url);
            if (getData != "")
            {
                this->SerialPico->println("[GET/SUCCESS] GET request successful.");
                this->SerialPico->println(getData);
                this->SerialPico->flush();
                this->SerialPico->println();
                this->SerialPico->println("[GET/END]");
            }
            else
            {
                this->SerialPico->println("[ERROR] GET request failed or returned empty data.");
            }
        }
        // Handle [GET/HTTP] command
        else if (_data.startsWith("[GET/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url.");
                this->ledOff();
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
            String getData = this->get(url, headerKeys, headerValues, headerSize);
            if (getData != "")
            {
                this->SerialPico->println("[GET/SUCCESS] GET request successful.");
                this->SerialPico->println(getData);
                this->SerialPico->flush();
                this->SerialPico->println();
                this->SerialPico->println("[GET/END]");
            }
            else
            {
                this->SerialPico->println("[ERROR] GET request failed or returned empty data.");
            }
        }
        // Handle [POST/HTTP] command
        else if (_data.startsWith("[POST/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
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
            String postData = this->post(url, payload, headerKeys, headerValues, headerSize);
            if (postData != "")
            {
                this->SerialPico->println("[POST/SUCCESS] POST request successful.");
                this->SerialPico->println(postData);
                this->SerialPico->flush();
                this->SerialPico->println();
                this->SerialPico->println("[POST/END]");
            }
            else
            {
                this->SerialPico->println("[ERROR] POST request failed or returned empty data.");
            }
        }
        // Handle [PUT/HTTP] command
        else if (_data.startsWith("[PUT/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[PUT/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
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
            String putData = this->put(url, payload, headerKeys, headerValues, headerSize);
            if (putData != "")
            {
                this->SerialPico->println("[PUT/SUCCESS] PUT request successful.");
                this->SerialPico->println(putData);
                this->SerialPico->flush();
                this->SerialPico->println();
                this->SerialPico->println("[PUT/END]");
            }
            else
            {
                this->SerialPico->println("[ERROR] PUT request failed or returned empty data.");
            }
        }
        // Handle [DELETE/HTTP] command
        else if (_data.startsWith("[DELETE/HTTP]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[DELETE/HTTP]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
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
            String deleteData = this->delete_request(url, payload, headerKeys, headerValues, headerSize);
            if (deleteData != "")
            {
                this->SerialPico->println("[DELETE/SUCCESS] DELETE request successful.");
                this->SerialPico->println(deleteData);
                this->SerialPico->flush();
                this->SerialPico->println();
                this->SerialPico->println("[DELETE/END]");
            }
            else
            {
                this->SerialPico->println("[ERROR] DELETE request failed or returned empty data.");
            }
        }
        // Handle [GET/BYTES]
        else if (_data.startsWith("[GET/BYTES]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[GET/BYTES]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url.");
                this->ledOff();
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
            if (!this->get_bytes_to_file(url, headerKeys, headerValues, headerSize))
            {
                this->SerialPico->println("[ERROR] GET request failed or returned empty data.");
            }
        }
        // handle [POST/BYTES]
        else if (_data.startsWith("[POST/BYTES]"))
        {
            if (!this->isConnectedToWifi() && !this->connectToWifi())
            {
                this->SerialPico->println("[ERROR] Not connected to Wifi. Failed to reconnect.");
                this->ledOff();
                return;
            }

            // Extract the JSON by removing the command part
            String jsonData = _data.substring(strlen("[POST/BYTES]"));
            jsonData.trim();

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("url") || !doc.containsKey("payload"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain url or payload.");
                this->ledOff();
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
            if (!this->post_bytes_to_file(url, payload, headerKeys, headerValues, headerSize))
            {
                this->SerialPico->println("[ERROR] POST request failed or returned empty data.");
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

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("key") || !doc.containsKey("json"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain key or json.");
                this->ledOff();
                return;
            }
            String key = doc["key"];
            JsonObject json = doc["json"];

            if (json.containsKey(key))
            {
                this->SerialPico->println(json[key].as<String>());
            }
            else
            {
                this->SerialPico->println("[ERROR] Key not found in JSON.");
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

            DynamicJsonDocument doc(1024);
            DeserializationError error = deserializeJson(doc, jsonData);

            if (error)
            {
                this->SerialPico->print("[ERROR] Failed to parse JSON.");
                this->ledOff();
                return;
            }

            // Extract values from JSON
            if (!doc.containsKey("key") || !doc.containsKey("index") || !doc.containsKey("json"))
            {
                this->SerialPico->println("[ERROR] JSON does not contain key, index, or json.");
                this->ledOff();
                return;
            }
            String key = doc["key"];
            int index = doc["index"];
            JsonArray json = doc["json"];

            if (json[index].containsKey(key))
            {
                this->SerialPico->println(json[index][key].as<String>());
            }
            else
            {
                this->SerialPico->println("[ERROR] Key not found in JSON.");
            }
        }

        this->ledOff();
    }
}
