/* FlipperHTTPVGM.h for flipper-http-vgm.ino (for the Video Game Module)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: Raspberry Pi Pico
Created: 2025-01-01
Updated: 2025-01-01
*/
#include "FlipperHTTPVGM.h"

// Clear serial buffer to avoid any residual data
void FlipperHTTP::clearSerialBuffer()
{
    while (this->SerialPico->available() > 0)
    {
        this->SerialPico->read();
    }
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
    this->SerialPico->flush();
}

String FlipperHTTP::readSerialLine()
{
    String receivedData = "";

    receivedData = this->SerialPico->readStringUntil('\n');

    receivedData.trim(); // Remove any leading/trailing whitespace

    return receivedData;
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
    }
}