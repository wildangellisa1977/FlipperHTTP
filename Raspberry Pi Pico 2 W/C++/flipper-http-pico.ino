/* FlipperHTTP Library (for the Raspberry Pi Pico 2 W)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: Raspberry Pi Pico W
Created: 2024-12-07
Updated: 2024-12-07
*/

#include "FlipperHTTPPico2W.h"

FlipperHTTP fhttp;

void setup()
{
    fhttp.setup();
}

void loop()
{
    fhttp.loop();
}