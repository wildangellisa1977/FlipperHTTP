/* FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: ESP32S2 Dev Module
Created: 2024-09-30
Updated: 2025-01-12
*/

#include <FlipperHTTP.h>

FlipperHTTP fhttp;

void setup()
{
  fhttp.setup();
}

void loop()
{
  fhttp.loop();
}