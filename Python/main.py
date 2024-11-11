"""
FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-30
Updated: 2024-11-10
"""

from FlipperHTTPPico import FlipperHTTP, led


fhttp = FlipperHTTP()

if __name__ == "__main__":
    fhttp.setup()
    try:
        fhttp.loop()
    except KeyboardInterrupt:
        led.off()
    except OSError as e:
        fhttp.ledStart()
        fhttp.saveError(e, True)
    except Exception as e:
        fhttp.ledStart()
        fhttp.saveError(e, False)
