/* FlipperHTTPVGM.h for flipper-http-vgm.ino (for the Video Game Module)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: Raspberry Pi Pico
Created: 2024-01-01
Updated: 2024-01-01

Change Log:
- 2024-01-01: Initial commit
*/
#pragma once

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

class FlipperHTTP
{
public:
    // Constructor
    FlipperHTTP()
    {
    }

    void clearSerialBuffer();
    void ledAction(int timeout = 250);             // Function to flash the LED
    void ledStart();                               // Display LED sequence when the Pico is first connected to the Flipper
    void ledStatus();                              // Starting LED (Green only)
    void ledOff() { digitalWrite(PICO_LED, OFF); } // Turn off LED
    void setup();                                  // Arduino setup function
    String readSerialLine();                       // Read serial data until newline character

    void loop(); // Main loop for flipper-http-vgm.ino that handles all of the commands
private:
    SerialPIO *SerialPico; // Serial object for the Pico
    bool useLED = true;    // Variable to control LED usage
};