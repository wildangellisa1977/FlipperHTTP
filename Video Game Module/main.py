"""
FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-12-31
Updated: 2024-12-31
"""

from FlipperHTTPVGM import FlipperHTTP, Pin

fhttp = FlipperHTTP()

if __name__ == "__main__":
    fhttp.setup()

    try:
        fhttp.loop()
    except KeyboardInterrupt:
        red_pin = Pin(24, Pin.OUT)  # from TX 1
        blue_pin = Pin(21, Pin.OUT)  # from RX 1
        red_pin.off()
        blue_pin.off()
        fhttp.led.off()
    except OSError as e:
        fhttp.ledStart()
    except Exception as e:
        fhttp.ledStart()
