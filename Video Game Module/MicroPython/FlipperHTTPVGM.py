""" FlipperHTTPVGM.py (for the Video Game Module)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-12-31
Updated: 2025-01-01

Change Log:
- 2024-12-31: Initial commit
- 2025-01-01: Removed ICM42688P
"""

from machine import UART, Pin
from time import sleep, ticks_ms
import gc


class FlipperHTTP:
    def __init__(self) -> FlipperHTTP:
        self.use_led = True
        self.timeout = 2000  # milliseconds
        self.uart_flipper = None
        self.uart_esp32 = None
        self.led = Pin("LED", Pin.OUT)  # LED on the Pico
        self.BAUD_RATE = 115200

    def setup(self) -> None:
        """Start UART and load the WiFi credentials"""
        self.uart_flipper = UART(0, baudrate=self.BAUD_RATE, tx=Pin(0), rx=Pin(1))
        self.uart_esp32 = UART(1, baudrate=self.BAUD_RATE, tx=Pin(24), rx=Pin(21))
        self.uart_flipper.init()
        self.uart_esp32.init()
        self.use_led = True
        self.ledStart()
        self.flush(self.uart_flipper)
        self.flush(self.uart_esp32)

    def ledAction(self, timeout: float = 0.250):
        """Function to flash the LED"""
        self.led.on()
        sleep(timeout)
        self.led.off()
        sleep(timeout)

    def ledStart(self):
        """Display LED sequence when Wifi Board is first connected to the Flipper"""
        self.ledAction()
        self.ledAction()
        self.ledAction()

    def ledStatus(self) -> None:
        if self.use_led:
            self.led.on()

    def clearSerialBuffer(uart) -> None:
        """Clear the serial buffer"""
        while uart.any() > 0:
            uart.read()

    def write(self, uart, message: bytes):
        uart.write(message)

    def println(self, uart, message: str):
        uart.write(message + "\n")

    def flush(self, uart):
        uart.flush()

    def readSerialLine(uart) -> str:
        data = ""
        try:
            raw_data = uart.read()
            if raw_data:  # Ensures raw_data isn't empty before decoding
                data = raw_data.decode()
        except Exception as e:
            pass  # raw_data is empty/None
        return data

    def readLine(self, uart) -> str:
        start_time = ticks_ms()
        message = ""

        while (ticks_ms() - start_time) < self.timeout:
            if uart.any() > 0:
                try:
                    raw_data = uart.read()
                    if raw_data:
                        # Reset the timeout when data is read
                        start_time = ticks_ms()
                        message += raw_data.decode()

                        if "\n" in message:
                            message = message.strip("\n")
                            return message
                except Exception as e:
                    continue

        # Timeout reached with no newline received
        return None

    # the Flipper only sends data to the ESP32, so if data is received from the Flipper,
    # it's meant for the ESP32. and if no data is received from the Flipper,
    # then continuously send sensor data to the Flipper
    def loop(self):
        while True:
            gc.collect()  # Run garbage collection to free up memory
            try:
                if self.uart_flipper.any() > 0:
                    data = self.readLine(self.uart_flipper)

                    self.ledStatus()

                    if not data:  # Checks for None or empty string
                        self.led.off()
                        continue

                    # data is more than likely valid, so send to ESP32
                    self.println(self.uart_esp32, data)

                    # Wait for response from ESP32
                    response = self.readLine(self.uart_esp32)

                    if not response:
                        self.led.off()
                        continue

                    # Send response back to Flipper
                    self.println(self.uart_flipper, response)

                    self.led.off()

                elif self.uart_esp32.any() > 0:
                    data = self.readLine(self.uart_esp32)

                    self.ledStatus()

                    if not data:  # Checks for None or empty string
                        self.led.off()
                        continue

                    # data is more than likely valid, so send to Flipper
                    self.println(self.uart_flipper, data)

            except Exception as e:
                self.println(self.uart_flipper, f"[ERROR] {e}")
                self.led.off()
                continue
