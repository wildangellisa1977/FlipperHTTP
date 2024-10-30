"""
FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-30
Updated: 2024-10-30
"""

from FlipperHTTPPico import FlipperHTTP, led

fhttp = FlipperHTTP()

if __name__ == "__main__":
    fhttp.setup()
    try:
        fhttp.loop()
    except KeyboardInterrupt:
        led.off()
    except Exception as e:
        print(f"Error: {e}")
        led.off()
