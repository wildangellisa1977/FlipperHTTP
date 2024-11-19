""" FlipperHTTPPico.py (for the Raspberry Pi Pico W)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-30
Updated: 2024-11-18

Change Log:
- 2024-10-30: Initial commit
- 2024-11-01:
    - Added HTTP/1.1 support
    - Improved handling of bytes in buffers
    - Implemented additional error handling
- 2024-11-03: Updated requests module
- 2024-11-07: Added garbage collection to free up memory and try-except block to handle exceptions
- 2024-11-10: Updated stream handling in GET and POST requests
- 2024-11-15:
    - Updated [GET/BYTES] and [POST/BYTES]
    - Fixed local variable referenced before assignment error
- 2024-11-18: Added SD card support
- 2024-11-19: Mount/Unmount SD card
"""

from machine import UART, Pin
import network
import ujson
from time import sleep, ticks_ms
import errno
import urequests_2 as requests  # for HTTP/1.1 support
import gc
from urequests_2 import RESPONSE_IS_BUSY
from EasySD import EasySD


class FlipperHTTP:
    def __init__(self) -> FlipperHTTP:
        self.local_ip = None
        self.wifi_ip = None
        self.wlan = network.WLAN(network.STA_IF)
        self.ssid = None
        self.password = None
        self.use_led = True
        self.timeout = 2000  # milliseconds
        self.uart = None
        self.led = Pin("LED", Pin.OUT)  # LED on the Pico W
        self.BAUD_RATE = 115200
        self.sd = None

        try:
            self.sd = EasySD(auto_mount=True)
        except Exception as e:
            self.saveError(e)

    def saveError(self, err, is_os_error: bool = False) -> None:
        if not is_os_error:
            if not self.sd:
                with open("error.txt", "w") as f:
                    f.write(f"Error: {err}")
            else:
                try:
                    self.sd.write("error.txt", f"Error: {err}")
                except Exception as e:
                    print(e)
                    with open("error.txt", "w") as f:
                        f.write(f"Error: {err}")
        else:
            if err.errno == errno.ENOENT:
                reason = "File or directory not found."
            elif err.errno == errno.EACCES:
                reason = "Permission denied."
            elif err.errno == errno.ECONNREFUSED:
                reason = "Connection refused."
            elif err.errno == errno.EPERM:
                reason = "Operation not permitted."
            else:
                reason = str(err)

            if not self.sd:
                with open("error.txt", "w") as f:
                    f.write(f"OSError: {reason}")
            else:
                try:
                    self.sd.write("error.txt", f"OSError: {reason}")
                except Exception as e:
                    print(e)
                    with open("error.txt", "w") as f:
                        f.write(f"OSError: {reason}")

    def setup(self) -> None:
        """Start UART and load the WiFi credentials"""
        self.uart = UART(0, baudrate=self.BAUD_RATE, tx=Pin(0), rx=Pin(1))
        self.uart.init()
        self.use_led = True
        self.ledStart()
        self.loadWifiSettings()
        self.flush()

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

    def clearSerialBuffer(self) -> None:
        """Clear the serial buffer"""
        while self.uart.any() > 0:
            self.uart.read()

    def connectToWiFi(self) -> bool:
        if self.ssid is None or self.password is None:
            self.println("[ERROR] WiFi SSID or Password is empty.")
            return False
        try:
            self.wlan.active(True)
            if not self.wlan.isconnected():
                self.wlan.connect(self.ssid, self.password)
                while not self.wlan.isconnected():
                    self.uart.write(".")
                    sleep(0.5)
            if self.wlan.isconnected():
                self.println("[SUCCESS] Successfully connected to Wifi.")
                self.local_ip = self.wlan.ifconfig()[0]
                return True
        except Exception as e:
            self.saveError(e)
            self.println("[ERROR] Failed to connect to Wifi.")
            return False

    def isConnectedToWiFi(self) -> bool:
        return self.wlan.isconnected()

    def saveWifiSettings(self, jsonData: str) -> bool:
        try:
            # Parse JSON data
            data = ujson.loads(jsonData)
            new_ssid = data.get("ssid")
            new_password = data.get("password")

            if not new_ssid or not new_password:
                self.println("[ERROR] SSID or Password is empty.")
                return False

            # Load existing settings
            try:
                if not self.sd:
                    with open("flipper-http.json", "r") as f:
                        settings = ujson.loads(f.read())
                else:
                    try:
                        settings = ujson.loads(self.sd.read("flipper-http.json"))
                    except Exception as e:
                        print(e)
                        with open("flipper-http.json", "r") as f:
                            settings = ujson.loads(f.read())
            except OSError as e:
                if e.errno == errno.ENOENT:
                    settings = {"wifi_list": []}  # Initialize if file doesn't exist
                else:
                    self.saveError(e, True)
                    return False
            except Exception as e:
                self.println(f"[ERROR] Failed to load existing settings: {e}")
                return False

            # Check if SSID already exists in the list
            found = False
            for wifi in settings.get("wifi_list", []):
                if wifi.get("ssid") == new_ssid:
                    found = True
                    break

            # Add new SSID and password if not found
            if not found:
                settings["wifi_list"].append(
                    {"ssid": new_ssid, "password": new_password}
                )

                # Save updated settings to file
                try:
                    if not self.sd:
                        with open("flipper-http.json", "w") as f:
                            f.write(ujson.dumps(settings))
                    else:
                        try:
                            self.sd.write("flipper-http.json", ujson.dumps(settings))
                        except Exception as e:
                            print(e)
                            with open("flipper-http.json", "w") as f:
                                f.write(ujson.dumps(settings))
                    self.println("[SUCCESS] Settings saved.")
                    self.ssid = new_ssid
                    self.password = new_password
                    return True
                except Exception as e:
                    self.println(f"[ERROR] Failed to open file for writing: {e}")
                    return False

            self.ssid = new_ssid
            self.password = new_password
            self.println("[INFO] SSID already exists in settings.")
            return True

        except ValueError:
            self.println("[ERROR] Failed to parse JSON data.")
            return False

    def loadWifiSettings(self) -> bool:
        try:
            # Open the settings file and read content
            if not self.sd:
                with open("flipper-http.json", "r") as f:
                    file_content = f.read()
            else:
                try:
                    file_content = self.sd.read("flipper-http.json")
                except Exception as e:
                    print(e)
                    with open("flipper-http.json", "r") as f:
                        file_content = f.read

            if not file_content:
                return False

            # Parse JSON content
            settings = ujson.loads(file_content)
            wifi_list = settings.get("wifi_list", [])

            # Try each WiFi configuration in the list
            for wifi in wifi_list:
                self.ssid = wifi.get("ssid")
                self.password = wifi.get("password")

                if not self.ssid or not self.password:
                    continue  # Skip if missing SSID or password

                self.connectToWiFi()

                # Retry connection for up to 4 attempts, with 500ms delay each
                attempts = 0
                while not self.isConnectedToWiFi() and attempts < 4:
                    sleep(0.5)
                    self.uart.write(".")
                    attempts += 1

                if self.isConnectedToWiFi():
                    return True
            return False

        except (OSError, ValueError) as e:
            self.saveError(f"Failed to load or parse settings file: {e}")
            return False

    def get(self, url, headers=None, stream=None, uart=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if headers:
            headers["User-Agent"] = "Pico"  # for FlipSocial app
            if not stream:
                return requests.get(url=url, headers=headers)
            return requests.get(url=url, headers=headers, stream=stream, uart=uart)
        headers = {"User-Agent": "Pico"}  # for FlipSocial app
        if not stream:
            return requests.get(url=url, headers=headers)
        return requests.get(url=url, stream=stream, uart=uart, headers=headers)

    def post(self, url, payload, headers=None, stream=None, uart=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            if headers:
                headers["User-Agent"] = "Pico"  # for FlipSocial app
                if not stream:
                    return requests.post(url, headers=headers, data=payload)
                return requests.post(
                    url, headers=headers, data=payload, stream=stream, uart=uart
                )
            headers = {"User-Agent": "Pico"}  # for FlipSocial app
            if not stream:
                return requests.post(url, data=payload, headers=headers)
            return requests.post(
                url, data=payload, stream=stream, uart=uart, headers=headers
            )
        if headers:
            headers["User-Agent"] = "Pico"  # for FlipSocial app
            if not stream:
                return requests.post(
                    url, headers=headers, json_data=ujson.dumps(payload)
                )
            return requests.post(
                url,
                headers=headers,
                json_data=ujson.dumps(payload),
                stream=stream,
                uart=uart,
            )
        headers = {"User-Agent": "Pico"}  # for FlipSocial app
        if not stream:
            return requests.post(url, json_data=ujson.dumps(payload), headers=headers)
        return requests.post(
            url,
            json_data=ujson.dumps(payload),
            stream=stream,
            uart=uart,
            headers=headers,
        )

    def put(self, url, payload, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            if headers:
                return requests.put(url, headers=headers, data=payload)
            return requests.put(url, data=payload)
        if headers:
            return requests.put(url, headers=headers, json_data=ujson.dumps(payload))
        return requests.put(url, json_data=ujson.dumps(payload))

    def delete(self, url, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if headers:
            return requests.delete(url, headers=headers)
        return requests.delete(url)

    def head(self, url, payload, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            if headers:
                return requests.head(url, headers=headers, data=payload)
            return requests.head(url, data=payload)
        if headers:
            return requests.head(url, headers=headers, json_data=ujson.dumps(payload))
        return requests.head(url, json_data=ujson.dumps(payload))

    def patch(self, url, payload, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            if headers:
                return requests.patch(url, headers=headers, data=payload)
            return requests.patch(url, data=payload)
        if headers:
            return requests.patch(url, headers=headers, json_data=ujson.dumps(payload))
        return requests.patch(url, json_data=ujson.dumps(payload))

    def write(self, message: bytes):
        self.uart.write(message)

    def println(self, message: str):
        self.uart.write(message + "\n")

    def flush(self):
        self.uart.flush()

    def readSerialLine(self) -> str:
        data = ""
        try:
            raw_data = self.uart.read()
            if raw_data:  # Ensures raw_data isn't empty before decoding
                data = raw_data.decode()
        except Exception as e:
            pass  # raw_data is empty/None
        return data

    def readLine(self) -> str:
        start_time = ticks_ms()
        message = ""

        while (ticks_ms() - start_time) < self.timeout:
            if self.uart.any() > 0:
                try:
                    raw_data = self.uart.read()
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

    def loop(self):
        """Main loop to handle the serial communication"""
        global RESPONSE_IS_BUSY
        while True:
            gc.collect()  # Run garbage collection to free up memory
            try:
                if self.uart.any() > 0:
                    data = self.readLine()

                    self.ledStatus()

                    if not data:  # Checks for None or empty string
                        self.led.off()
                        continue

                    if data.startswith("[LIST]"):
                        self.println(
                            "[LIST],[PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS]"
                        )

                    # handle [LED/ON] command
                    elif data.startswith("[LED/ON]"):
                        self.use_led = True

                    # handle [LED/OFF] command
                    elif data.startswith("[LED/OFF]"):
                        self.use_led = False

                    # handle [IP/ADDRESS] command (local IP)
                    elif data.startswith("[IP/ADDRESS]"):
                        self.println(self.local_ip)

                    # handle [WIFI/IP] command (wifi IP)
                    elif data.startswith("[WIFI/IP]"):
                        res = self.get("https://ipwhois.app/json/")
                        if res is not None:
                            self.wifi_ip = res.json()["ip"]
                            self.println(self.wifi_ip)
                        else:
                            self.println(
                                "[ERROR] GET request failed or returned empty data."
                            )

                    # Ping/Pong to see if board/flipper is connected
                    elif data.startswith("[PING]"):
                        self.println("[PONG]")

                    # Handle [REBOOT] command
                    elif data.startswith("[REBOOT]"):
                        machine.reset()

                    # scan for wifi networks
                    elif data.startswith("[WIFI/SCAN]"):
                        try:
                            ap = network.WLAN(network.AP_IF)
                            ap.active(True)
                            networks = ap.scan()
                            network_data = []
                            for w in networks:
                                network_data.append(w[0].decode())
                            self.println(str(network_data))
                            ap.active(False)
                        except Exception as e:
                            self.saveError(e)
                            self.println("[ERROR] Failed to scan for WiFi networks.")

                    # Handle [WIFI/SAVE] command
                    elif data.startswith("[WIFI/SAVE]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[WIFI/SAVE]", "")
                        if self.saveWifiSettings(json_data):
                            self.println("[SUCCESS] Saved WiFi settings.")
                        else:
                            self.println("[ERROR] Failed to save WiFi settings.")

                    # Handle [WIFI/CONNECT] command
                    elif data.startswith("[WIFI/CONNECT]"):
                        if not self.isConnectedToWiFi():
                            self.connectToWiFi()
                            if self.isConnectedToWiFi():
                                self.println("[SUCCESS] Connected to WiFi")
                            else:
                                self.println("[ERROR] Failed to connect to Wifi.")
                        else:
                            self.println("[INFO] Already connected to WiFi")

                    # Handle [WIFI/DISCONNECT] command
                    elif data.startswith("[WIFI/DISCONNECT]"):
                        self.wlan.disconnect()
                        self.println("[DISCONNECTED] Wifi has been disconnected.")

                    # Handle [GET] command
                    elif data.startswith("[GET]"):
                        # Extract URL by removing the command part
                        url = data.replace("[GET]", "")
                        res = self.get(url)
                        if res is not None:
                            self.println("[GET/SUCCESS] GET request successful.")
                            self.println(res.text)
                            self.flush()
                            self.println("[GET/END]")
                        else:
                            self.println(
                                "[ERROR] GET request failed or returned empty data."
                            )

                    # Handle [GET/HTTP] command
                    elif data.startswith("[GET/HTTP]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[GET/HTTP]", "")
                        data = ujson.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        if not url:
                            self.println("[ERROR] JSON does not contain url.")
                            self.led.off()
                            return  # Exit the handler if URL is missing
                        if not headers:
                            headers = None
                        res = self.get(data["url"], data["headers"])
                        if res is not None:
                            self.println("[GET/SUCCESS] GET request successful.")
                            self.println(res.text)
                            self.flush()
                            self.println("[GET/END]")
                        else:
                            self.println(
                                "[ERROR] GET request failed or returned empty data."
                            )

                    # Handle [POST/HTTP] command
                    elif data.startswith("[POST/HTTP]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[POST/HTTP]", "")
                        data = ujson.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        payload = data["payload"]
                        if not url or not payload:
                            self.println(
                                "[ERROR] JSON does not contain url or payload."
                            )
                            self.led.off()
                            return  # Exit the handler if URL is missing
                        if not headers:
                            headers = None
                        res = self.post(url, payload, headers)
                        if res is not None:
                            self.println("[POST/SUCCESS] POST request successful.")
                            self.println(res.text)
                            self.flush()
                            self.println("[POST/END]")
                        else:
                            self.println(
                                "[ERROR] POST request failed or returned empty data."
                            )

                    # Handle [PUT/HTTP] command
                    elif data.startswith("[PUT/HTTP]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[PUT/HTTP]", "")
                        data = ujson.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        payload = data["payload"]
                        if not url or not payload:
                            self.println(
                                "[ERROR] JSON does not contain url or payload."
                            )
                            self.led.off()
                            return  # Exit the handler if URL is missing
                        if not headers:
                            headers = None
                        res = self.put(url, payload, headers)
                        if res is not None:
                            self.println("[PUT/SUCCESS] PUT request successful.")
                            self.println(res.text)
                            self.flush()
                            self.println("[PUT/END]")
                        else:
                            self.println(
                                "[ERROR] PUT request failed or returned empty data."
                            )

                    # Handle [DELETE/HTTP] command
                    elif data.startswith("[DELETE/HTTP]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[DELETE/HTTP]", "")
                        data = ujson.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        if not url:
                            self.println("[ERROR] JSON does not contain url.")
                            self.led.off()
                            return  # Exit the handler if URL is missing
                        if not headers:
                            headers = None
                        res = self.delete(url, headers)
                        if res is not None:
                            self.println("[DELETE/SUCCESS] DELETE request successful.")
                            self.println(res.text)
                            self.flush()
                            self.println("[DELETE/END]")
                        else:
                            self.println(
                                "[ERROR] DELETE request failed or returned empty data."
                            )

                    # Handle [GET/BYTES] command
                    elif data.startswith("[GET/BYTES]"):
                        try:
                            # Extract the JSON by removing the command part
                            json_data = data.replace("[GET/BYTES]", "")
                            data = ujson.loads(json_data)

                            url = data["url"]
                            headers = data.get("headers", None)

                            if not url:
                                self.println("[ERROR] JSON does not contain url.")
                                self.led.off()
                                return  # Exit the handler if URL is missing

                            # Initialize the response variable
                            res = None

                            # Make the GET request
                            gc.collect()
                            try:
                                if not headers:
                                    res = self.get(url=url, stream=True, uart=self.uart)
                                else:
                                    res = self.get(
                                        url=url,
                                        headers=headers,
                                        stream=True,
                                        uart=self.uart,
                                    )
                            except Exception as e:
                                self.saveError(e)
                                self.led.off()
                                return

                            if res is not None:
                                if res.status_code >= 400:
                                    self.println(
                                        f"[ERROR] GET request failed: {res.status_code}"
                                    )
                                    self.led.off()
                                    return

                                # Wait until res finishes sending uart
                                while RESPONSE_IS_BUSY is True:
                                    pass

                                return
                            else:
                                self.println(
                                    "[ERROR] GET request failed or returned empty data."
                                )

                        except (ValueError, KeyError) as e:
                            self.println(f"[ERROR] Failed to parse JSON: {e}")
                        except Exception as e:
                            self.saveError(e)
                            self.println(f"[ERROR] GET request failed: {e}")

                    # Handle [POST/BYTES] command
                    elif data.startswith("[POST/BYTES]"):
                        try:
                            # Extract the JSON by removing the command part
                            json_data = data.replace("[POST/BYTES]", "")
                            data = ujson.loads(json_data)

                            url = data["url"]
                            headers = data.get("headers", None)
                            payload = data.get("payload", None)

                            if not url or payload is None:
                                self.println(
                                    "[ERROR] JSON does not contain url or payload."
                                )
                                self.led.off()
                                return

                            # Initialize the response variable
                            res = None

                            # Make the POST request
                            gc.collect()
                            try:
                                if not headers:
                                    res = self.post(
                                        url, payload, stream=True, uart=self.uart
                                    )
                                else:
                                    res = self.post(
                                        url=url,
                                        payload=payload,
                                        headers=headers,
                                        stream=True,
                                        uart=self.uart,
                                    )
                            except Exception as e:
                                self.saveError(e)
                                self.led.off()
                                return

                            if res is not None:
                                if res.status_code >= 400:
                                    self.println(
                                        f"[ERROR] POST request failed: {res.status_code}"
                                    )
                                    self.led.off()
                                    return

                                # Wait until res finishes sending uart
                                while RESPONSE_IS_BUSY is True:
                                    pass

                                return
                            else:
                                self.println(
                                    "[ERROR] POST request failed or returned empty data."
                                )

                        except (ValueError, KeyError) as e:
                            self.println(f"[ERROR] Failed to parse JSON: {e}")
                        except Exception as e:
                            self.saveError(e)
                            self.println(f"[ERROR] POST request failed: {e}")

                    # Handle [PARSE] command
                    elif data.startswith("[PARSE]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[PARSE]", "")
                        data = ujson.loads(json_data)
                        json = data["json"]
                        key = data["key"]
                        if not json or not key:
                            self.println("[ERROR] JSON does not contain key or json.")
                            self.led.off()
                            return
                        res = ujson.loads(json)
                        if res is not None:
                            found_key = res.get(key)
                            if found_key:
                                self.println(found_key)
                            else:
                                self.println("[ERROR] Key not found in JSON.")
                        else:
                            self.println(
                                "[ERROR] JSON parsing failed or key not found."
                            )

                    # Handle [PARSE/ARRAY] command
                    elif data.startswith("[PARSE/ARRAY]"):
                        # Extract the JSON by removing the command part
                        json_data = data.replace("[PARSE/ARRAY]", "")
                        data = ujson.loads(json_data)
                        json = data["json"]
                        key = data["key"]
                        index = data["index"]
                        if not json or not key or not index:
                            self.println(
                                "[ERROR] JSON does not contain key, index or json."
                            )
                            self.led.off()
                            return
                        res = ujson.loads(json)
                        if res is not None:
                            found_key = res.get(key)
                            if found_key:
                                self.println(found_key[index])
                            else:
                                self.println("[ERROR] Key not found in JSON.")
                        else:
                            self.println(
                                "[ERROR] JSON parsing failed or key not found."
                            )

                    self.led.off()
            except Exception as e:
                self.saveError(e)
                self.println(f"[ERROR] {e}")
                self.led.off()
                continue
