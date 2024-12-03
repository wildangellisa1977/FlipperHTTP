/* FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: ESP32 Wrover Module
Created: 2024-09-30
Updated: 2024-12-03
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