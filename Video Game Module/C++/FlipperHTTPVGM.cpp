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
void FlipperHTTP::clearSerialBuffer(SerialPIO *serial)
{
    while (serial->available() > 0)
    {
        serial->read();
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
    this->SerialFlipper = new SerialPIO(0, 1); // TX on GPIO0, RX on GPIO1
    this->SerialESP32 = new SerialPIO(24, 21); // TX on GPIO24, RX on GPIO21

    this->SerialFlipper->begin(BAUD_RATE);
    this->SerialESP32->begin(BAUD_RATE);

    this->SerialFlipper->setTimeout(5000);
    this->SerialESP32->setTimeout(5000);

    this->useLED = true;
    this->ledStart();
    this->SerialFlipper->flush();
    this->SerialESP32->flush();
}

String FlipperHTTP::readSerialLine(SerialPIO *serial)
{
    String receivedData = serial->readStringUntil('\n');

    receivedData.trim(); // Remove any leading/trailing whitespace and newline characters

    return receivedData;
}

// Main loop for flipper-http.ino that handles all of the commands
void FlipperHTTP::loop()
{
    // Check if there's incoming serial data
    if (this->SerialFlipper->available() > 0)
    {
        this->ledStatus();

        // Read the incoming serial data until newline
        String _data = this->readSerialLine(this->SerialFlipper);

        // send to ESP32
        this->SerialESP32->println(_data);

        // Wait for response from ESP32
        String _response = this->readSerialLine(this->SerialESP32);

        // Send response back to Flipper
        this->SerialFlipper->println(_response);

        this->ledOff();
    }
    else if (this->SerialESP32->available() > 0)
    {
        this->ledStatus();

        // Read the incoming serial data until newline
        String _data = this->readSerialLine(this->SerialESP32);

        // send to Flipper
        this->SerialFlipper->println(_data);

        this->ledOff();
    }
}