"""
FlipperHTTP Library
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-12-31
Updated: 2024-12-31
"""

from ICM42688P import ICM42688P
from FlipperHTTPVGM import FlipperHTTP, Pin
import time

# Initialize the sensor
sensor = ICM42688P()
sensor.initialize()
fhttp = FlipperHTTP()

try:
    fhttp.setup()

    # Example usage
    while True:
        accel = sensor.read_accelerometer()
        gyro = sensor.read_gyroscope()
        temp = sensor.read_temperature()
        print(f"Accel: {accel} G")
        print(f"Gyro: {gyro} DPS")
        print(f"Temp: {temp} °C")
        time.sleep(1)
except KeyboardInterrupt:
    print("Keyboard Interrupt")
finally:
    red_pin = Pin(24, Pin.OUT)  # from TX 1
    blue_pin = Pin(21, Pin.OUT)  # from RX 1
    red_pin.off()
    blue_pin.off()
    fhttp.led.off()
