/* FlipperHTTPPico.h for flipper-http-pico.ino (for the Rasberry Pi Pico W)
Author: JBlanked
Github: https://github.com/jblanked/WebCrawler-FlipperZero/tree/main/assets/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-24
Updated: 2024-11-09

Change Log:
- 2024-10-24: Initial commit
- 2024-10-25: Updated the readSerialLine method to use readStringUntil instead of reading char by char, and added Auto connect
  - Reading char by char worked on the dev board but was missing chars on the Pico
- 2024-10-26: Updated the saveWifiSettings and loadWifiSettings methods to save and load a list of wifi networks, and added [WIFI/LIST] command
- 2024-11-09: Added SSL certificate from https://letsencrypt.org/certificates/
*/

#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <LittleFS.h> // replacement for SPIFFS

// Define UART parameters
#define BAUD_RATE 115200
#define PICO_LED LED_BUILTIN
#define ON HIGH
#define OFF LOW

SerialPIO SerialPico(0, 1);

class FlipperHTTP
{
public:
  // Constructor
  FlipperHTTP()
  {
  }

  // Main methods for flipper-http.ino
  void loop();
  void setup()
  {
    SerialPico.begin(BAUD_RATE);

    if (!LittleFS.begin())
    {
      SerialPico.println("An Error has occurred while mounting LittleFS. Attempting to format...");
      if (LittleFS.format())
      {
        SerialPico.println("File system formatted successfully. Re-mounting...");
        if (!LittleFS.begin())
        {
          SerialPico.println("Failed to re-mount LittleFS after formatting.");
          delay(1000);
          return;
        }
      }
      else
      {
        SerialPico.println("File system formatting failed.");
        delay(1000);
        return;
      }
    }
    this->useLED = true;
    this->ledStart();
    this->loadWifiSettings();
    this->client.setCACert(this->root_ca);
    SerialPico.flush();
  }

  // HTTP Methods
  String get(String url);
  String get(String url, const char *headerKeys[], const char *headerValues[], int headerSize);
  String post(String url, String payload);
  String post(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);
  String put(String url, String payload);
  String put(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);
  String delete_request(String url, String payload);
  String delete_request(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);

  // stream data as bytes
  bool get_bytes_to_file(String url, const char *headerKeys[], const char *headerValues[], int headerSize);
  bool post_bytes_to_file(String url, String payload, const char *headerKeys[], const char *headerValues[], int headerSize);

  // Save and Load settings to and from SPIFFS
  bool saveWifiSettings(String data);
  bool loadWifiSettings();

  // returns a string of all wifi networks
  String scanWifiNetworks()
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

  // Connect to Wifi using the loaded SSID and Password
  bool connectToWifi();

  // Check if the Dev Board is connected to Wifi
  bool isConnectedToWifi() { return WiFi.status() == WL_CONNECTED; }

  // Read serial data until newline character
  String readSerialLine();

  // Clear serial buffer to avoid any residual data
  void clearSerialBuffer()
  {
    while (SerialPico.available() > 0)
    {
      SerialPico.read();
    }
  }

  // Function to flash the LED
  void ledAction(int timeout = 250)
  {
    digitalWrite(PICO_LED, ON);
    delay(timeout);
    digitalWrite(PICO_LED, OFF);
    delay(timeout);
  }

  // Display LED sequence when Wifi Board is first connected to the Flipper
  void ledStart()
  {
    // Initialize LED pin
    pinMode(PICO_LED, OUTPUT);

    ledAction();
    ledAction();
    ledAction();
  }

  // Starting LED (Green only)
  void ledStatus()
  {
    if (this->useLED)
    {
      digitalWrite(PICO_LED, ON);
    }
  }

  // Turn off all LEDs
  void ledOff()
  {
    digitalWrite(PICO_LED, OFF);
  }

  // get IP addresss
  String getIPAddress()
  {
    return WiFi.localIP().toString();
  }

private:
  const char *settingsFilePath = "/flipper-http.json"; // Path to the settings file in the SPIFFS file system
  char loadedSSID[64] = {0};                           // Variable to store SSID
  char loadedPassword[64] = {0};                       // Variable to store password
  bool useLED = true;                                  // Variable to control LED usage

  bool readSerialSettings(String receivedData, bool connectAfterSave);

  WiFiClientSecure client;

  // Root CA from letsencrypt
  // get it here: https://letsencrypt.org/certificates/
  const char *root_ca =
      "-----BEGIN CERTIFICATE-----\n"
      "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\n"
      "-----END CERTIFICATE-----\n";
};

//  Connect to Wifi using the loaded SSID and Password
bool FlipperHTTP::connectToWifi()
{
  if (String(loadedSSID) == "" || String(loadedPassword) == "")
  {
    SerialPico.println("[ERROR] WiFi SSID or Password is empty.");
    return false;
  }

  WiFi.disconnect(true); // Ensure WiFi is disconnected before reconnecting
  WiFi.begin(loadedSSID, loadedPassword);

  int i = 0;
  while (!this->isConnectedToWifi() && i < 20)
  {
    delay(500);
    i++;
    SerialPico.print(".");
  }
  SerialPico.println(); // Move to next line after dots

  if (this->isConnectedToWifi())
  {
    SerialPico.println("[SUCCESS] Successfully connected to Wifi.");
    return true;
  }
  else
  {
    SerialPico.println("[ERROR] Failed to connect to Wifi.");
    return false;
  }
}

// Save WiFi settings to LittleFS
bool FlipperHTTP::saveWifiSettings(String jsonData)
{
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, jsonData);

  if (error)
  {
    SerialPico.println("[ERROR] Failed to parse JSON data.");
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
      SerialPico.println("[ERROR] Failed to open file for writing.");
      return false;
    }

    serializeJson(existingDoc, file);
    file.close();
  }

  SerialPico.println("[SUCCESS] Settings saved to LittleFS.");
  return true;
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
      SerialPico.print(".");
    }

    if (this->isConnectedToWifi())
    {
      return true;
    }
  }
  return false;
}
String FlipperHTTP::readSerialLine()
{
  String receivedData = "";

  // this worked on dev board but even with delay changes, it was missing chars
  // while (SerialPico.available() > 0)
  // {
  //   char incomingChar = SerialPico.read();
  //   if (incomingChar == '\n')
  //   {
  //     break;
  //   }
  //   receivedData += incomingChar;
  //   delay(5); // Minimal delay to allow buffer to fill
  // }
  receivedData = SerialPico.readStringUntil('\n');

  receivedData.trim(); // Remove any leading/trailing whitespace

  return receivedData;
}

bool FlipperHTTP::readSerialSettings(String receivedData, bool connectAfterSave)
{
  DynamicJsonDocument doc(1024);
  DeserializationError error = deserializeJson(doc, receivedData);

  if (error)
  {
    SerialPico.print("[ERROR] Failed to parse JSON: ");
    SerialPico.println(error.c_str());
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
    SerialPico.println("[ERROR] JSON does not contain ssid and password.");
    return false;
  }

  // Save to SPIFFS
  if (!this->saveWifiSettings(receivedData))
  {
    SerialPico.println("[ERROR] Failed to save settings to file.");
    return false;
  }

  // Attempt to reconnect with new settings
  if (connectAfterSave && this->connectToWifi())
  {
    SerialPico.println("[SUCCESS] Connected to the new Wifi network.");
  }

  return true;
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] GET Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] GET Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] GET Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] GET Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }

  // Clear serial buffer to avoid any residual data
  this->clearSerialBuffer();

  return payload;
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] DELETE Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] DELETE Request Failed");
          }
        }
      }
    }
    http.end();
  }

  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] DELETE Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] DELETE Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }

  // Clear serial buffer to avoid any residual data
  this->clearSerialBuffer();

  return response;
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] POST Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] POST Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] POST Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] POST Request Failed");
          }
        }
      }
    }
    http.end();
  }

  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }

  // Clear serial buffer to avoid any residual data
  this->clearSerialBuffer();

  return response;
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] PUT Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] PUT Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
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
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.print("[ERROR] PUT Request Failed, error: ");
        SerialPico.println(error);
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
            SerialPico.println("[ERROR] PUT Request Failed");
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }

  // Clear serial buffer to avoid any residual data
  this->clearSerialBuffer();

  return response;
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
      SerialPico.println("[GET/SUCCESS]");

      int len = http.getSize();
      uint8_t buff[512] = {0};

      WiFiClient *stream = http.getStreamPtr();

      // Stream data while connected and available
      while (http.connected() && (len > 0 || len == -1))
      {
        size_t size = stream->available();
        if (size)
        {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          SerialPico.write(buff, c); // Write data to serial
          if (len > 0)
          {
            len -= c;
          }
        }
        delay(1); // Yield control to the system
      }

      // Flush the serial buffer to ensure all data is sent
      http.end();
      SerialPico.flush();
      SerialPico.println();
      SerialPico.println("[GET/END]");

      return true;
    }
    else
    {
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.printf("[ERROR] GET request failed with error: %s\n", error.c_str());
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
            int len = http.getSize();
            uint8_t buff[512] = {0};

            WiFiClient *stream = http.getStreamPtr();

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
              size_t size = stream->available();
              if (size)
              {
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                SerialPico.write(buff, c); // Write data to serial
                if (len > 0)
                {
                  len -= c;
                }
              }
              delay(1); // Yield control to the system
            }

            // Flush the serial buffer to ensure all data is sent
            http.end();
            SerialPico.flush();
            SerialPico.println();
            SerialPico.println("[GET/END]");
            this->client.setCACert(this->root_ca); // reset to secure
            return true;
          }
          else
          {
            this->client.setCACert(this->root_ca); // reset to secure
            SerialPico.printf("[ERROR] GET request failed with error: %s\n", http.errorToString(newCode).c_str());
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }
  return false;
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
      SerialPico.println("[POST/SUCCESS]");

      int len = http.getSize(); // Get the response content length
      uint8_t buff[512] = {0};  // Buffer for reading data

      WiFiClient *stream = http.getStreamPtr();

      // Stream data while connected and available
      while (http.connected() && (len > 0 || len == -1))
      {
        size_t size = stream->available();
        if (size)
        {
          int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
          SerialPico.write(buff, c); // Write data to serial
          if (len > 0)
          {
            len -= c;
          }
        }
        delay(1); // Yield control to the system
      }

      http.end();
      // Flush the serial buffer to ensure all data is sent
      SerialPico.flush();
      SerialPico.println();
      SerialPico.println("[POST/END]");

      return true;
    }
    else
    {
      String error = http.errorToString(httpCode).c_str();
      if (error != "connection failed")
      {
        SerialPico.printf("[ERROR] POST request failed with error: %s\n", error.c_str());
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
            int len = http.getSize(); // Get the response content length
            uint8_t buff[512] = {0};  // Buffer for reading data

            WiFiClient *stream = http.getStreamPtr();

            // Stream data while connected and available
            while (http.connected() && (len > 0 || len == -1))
            {
              size_t size = stream->available();
              if (size)
              {
                int c = stream->readBytes(buff, ((size > sizeof(buff)) ? sizeof(buff) : size));
                SerialPico.write(buff, c); // Write data to serial
                if (len > 0)
                {
                  len -= c;
                }
              }
              delay(1); // Yield control to the system
            }

            http.end();
            // Flush the serial buffer to ensure all data is sent
            SerialPico.flush();
            SerialPico.println();
            SerialPico.println("[POST/END]");
            this->client.setCACert(this->root_ca); // reset to secure
            return true;
          }
          else
          {
            this->client.setCACert(this->root_ca); // reset to secure
            SerialPico.printf("[ERROR] POST request failed with error: %s\n", http.errorToString(newCode).c_str());
          }
        }
      }
    }
    http.end();
  }
  else
  {
    SerialPico.println("[ERROR] Unable to connect to the server.");
  }
  return false;
}

// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
  // Check if there's incoming serial data
  if (SerialPico.available() > 0)
  {
    // Read the incoming serial data until newline
    String _data = this->readSerialLine();

    this->ledStatus();

    // print the available commands
    if (_data.startsWith("[LIST]"))
    {
      SerialPico.println("[LIST],[PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS]");
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
      SerialPico.println(this->getIPAddress());
    }
    // handle [WIFI/IP] command ip of connected wifi
    else if (_data.startsWith("[WIFI/IP]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
        this->ledOff();
        return;
      }
      // Get Request
      String jsonData = this->get("https://httpbin.org/get");
      if (jsonData == "")
      {
        SerialPico.println("[ERROR] GET request failed or returned empty data.");
        return;
      }
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, jsonData);
      if (error)
      {
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }
      if (!doc.containsKey("origin"))
      {
        SerialPico.println("[ERROR] JSON does not contain origin.");
        this->ledOff();
        return;
      }
      SerialPico.println(doc["origin"].as<String>());
    }
    // Ping/Pong to see if board/flipper is connected
    else if (_data.startsWith("[PING]"))
    {
      SerialPico.println("[PONG]");
    }
    // Handle [REBOOT] command
    else if (_data.startsWith("[REBOOT]"))
    {
      this->useLED = true;
      // ESP.restart();
    }
    // scan for wifi networks
    else if (_data.startsWith("[WIFI/SCAN]"))
    {
      SerialPico.println(this->scanWifiNetworks());
      SerialPico.flush();
    }
    // Handle Wifi list command
    else if (_data.startsWith("[WIFI/LIST]"))
    {
      File file = LittleFS.open(settingsFilePath, "r");
      if (!file)
      {
        SerialPico.println("[ERROR] Failed to open file for reading.");
        return;
      }

      String fileContent = file.readString();
      file.close();

      SerialPico.println(fileContent);
      SerialPico.flush();
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
        SerialPico.println("[SUCCESS] Wifi settings saved.");
      }
      else
      {
        SerialPico.println("[ERROR] Failed to save Wifi settings.");
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
          SerialPico.println("[SUCCESS] Connected to Wifi.");
        }
        else
        {
          SerialPico.println("[ERROR] Failed to connect to Wifi.");
        }
      }
      else
      {
        SerialPico.println("[INFO] Already connected to Wifi.");
      }
    }
    // Handle [WIFI/DISCONNECT] command
    else if (_data == "[WIFI/DISCONNECT]")
    {
      WiFi.disconnect(true);
      SerialPico.println("[DISCONNECTED] Wifi has been disconnected.");
    }
    // Handle [GET] command
    else if (_data.startsWith("[GET]"))
    {

      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.println("[GET/SUCCESS] GET request successful.");
        SerialPico.println(getData);
        SerialPico.flush();
        SerialPico.println();
        SerialPico.println("[GET/END]");
      }
      else
      {
        SerialPico.println("[ERROR] GET request failed or returned empty data.");
      }
    }
    // Handle [GET/HTTP] command
    else if (_data.startsWith("[GET/HTTP]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url"))
      {
        SerialPico.println("[ERROR] JSON does not contain url.");
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
        SerialPico.println("[GET/SUCCESS] GET request successful.");
        SerialPico.println(getData);
        SerialPico.flush();
        SerialPico.println();
        SerialPico.println("[GET/END]");
      }
      else
      {
        SerialPico.println("[ERROR] GET request failed or returned empty data.");
      }
    }
    // Handle [POST/HTTP] command
    else if (_data.startsWith("[POST/HTTP]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url") || !doc.containsKey("payload"))
      {
        SerialPico.println("[ERROR] JSON does not contain url or payload.");
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
        SerialPico.println("[POST/SUCCESS] POST request successful.");
        SerialPico.println(postData);
        SerialPico.flush();
        SerialPico.println();
        SerialPico.println("[POST/END]");
      }
      else
      {
        SerialPico.println("[ERROR] POST request failed or returned empty data.");
      }
    }
    // Handle [PUT/HTTP] command
    else if (_data.startsWith("[PUT/HTTP]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url") || !doc.containsKey("payload"))
      {
        SerialPico.println("[ERROR] JSON does not contain url or payload.");
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
        SerialPico.println("[PUT/SUCCESS] PUT request successful.");
        SerialPico.println(putData);
        SerialPico.flush();
        SerialPico.println();
        SerialPico.println("[PUT/END]");
      }
      else
      {
        SerialPico.println("[ERROR] PUT request failed or returned empty data.");
      }
    }
    // Handle [DELETE/HTTP] command
    else if (_data.startsWith("[DELETE/HTTP]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url") || !doc.containsKey("payload"))
      {
        SerialPico.println("[ERROR] JSON does not contain url or payload.");
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
        SerialPico.println("[DELETE/SUCCESS] DELETE request successful.");
        SerialPico.println(deleteData);
        SerialPico.flush();
        SerialPico.println();
        SerialPico.println("[DELETE/END]");
      }
      else
      {
        SerialPico.println("[ERROR] DELETE request failed or returned empty data.");
      }
    }
    // Handle [GET/BYTES]
    else if (_data.startsWith("[GET/BYTES]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url"))
      {
        SerialPico.println("[ERROR] JSON does not contain url.");
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
        SerialPico.println("[ERROR] GET request failed or returned empty data.");
      }
    }
    // handle [POST/BYTES]
    else if (_data.startsWith("[POST/BYTES]"))
    {
      if (!this->isConnectedToWifi() && !this->connectToWifi())
      {
        SerialPico.println("[ERROR] Not connected to Wifi. Failed to reconnect.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("url") || !doc.containsKey("payload"))
      {
        SerialPico.println("[ERROR] JSON does not contain url or payload.");
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
        SerialPico.println("[ERROR] POST request failed or returned empty data.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("key") || !doc.containsKey("json"))
      {
        SerialPico.println("[ERROR] JSON does not contain key or json.");
        this->ledOff();
        return;
      }
      String key = doc["key"];
      JsonObject json = doc["json"];

      if (json.containsKey(key))
      {
        SerialPico.println(json[key].as<String>());
      }
      else
      {
        SerialPico.println("[ERROR] Key not found in JSON.");
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
        SerialPico.print("[ERROR] Failed to parse JSON.");
        this->ledOff();
        return;
      }

      // Extract values from JSON
      if (!doc.containsKey("key") || !doc.containsKey("index") || !doc.containsKey("json"))
      {
        SerialPico.println("[ERROR] JSON does not contain key, index, or json.");
        this->ledOff();
        return;
      }
      String key = doc["key"];
      int index = doc["index"];
      JsonArray json = doc["json"];

      if (json[index].containsKey(key))
      {
        SerialPico.println(json[index][key].as<String>());
      }
      else
      {
        SerialPico.println("[ERROR] Key not found in JSON.");
      }
    }

    this->ledOff();
  }
}
