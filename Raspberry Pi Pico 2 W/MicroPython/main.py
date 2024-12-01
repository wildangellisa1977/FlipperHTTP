"""
FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-12-01
Updated: 2024-12-01
"""

from FlipperHTTPPico2W import FlipperHTTP


fhttp = FlipperHTTP()

if __name__ == "__main__":
    fhttp.setup()
    try:
        fhttp.loop()
    except KeyboardInterrupt:
        if fhttp.sd and fhttp.sd.is_mounted:
            try:
                fhttp.sd.unmount()
            except Exception as e:
                pass
        fhttp.led.off()
    except OSError as e:
        fhttp.ledStart()
        fhttp.saveError(e, True)
    except Exception as e:
        fhttp.ledStart()
        fhttp.saveError(e, False)
