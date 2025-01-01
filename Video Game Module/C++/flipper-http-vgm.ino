/* FlipperHTTP Library (for the Video Game Module)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Board Manager: Raspberry Pi Pico
Created: 2025-01-01
Updated: 2025-01-01
*/

#include "FlipperHTTPVGM.h"

FlipperHTTP fhttp;

void setup()
{
    fhttp.setup();
}

void loop()
{
    fhttp.loop();
}