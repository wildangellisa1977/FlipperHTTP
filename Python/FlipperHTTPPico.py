""" FlipperHTTPPico.py (for the Raspberry Pi Pico W)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-10-30
Updated: 2024-10-30

Change Log:
- 2024-10-30: Initial commit
"""

from machine import UART, Pin
import network
import urequests as requests
import ujson
from time import sleep

led = Pin("LED", Pin.OUT)  # LED on the Pico W
BAUD_RATE = 115200


class FlipperHTTP:
    def __init__(self) -> FlipperHTTP:
        self.local_ip = None
        self.wifi_ip = None
        self.wlan = network.WLAN(network.STA_IF)
        self.ssid = None
        self.password = None
        self.use_led = True

    def setup(self) -> None:
        """Start UART and load the WiFi credentials"""
        self.uart = UART(0, baudrate=BAUD_RATE, tx=Pin(0), rx=Pin(1))
        self.use_led = True
        self.ledStart()
        self.loadWifiSettings()
        self.uart.flush()

    def ledAction(self, timeout: float = 0.250):
        """Function to flash the LED"""
        led.on()
        sleep(timeout)
        led.off()
        sleep(timeout)

    def ledStart(self):
        """Display LED sequence when Wifi Board is first connected to the Flipper"""
        self.ledAction()
        self.ledAction()
        self.ledAction()

    def ledStatus(self) -> None:
        if self.use_led:
            led.on()

    def connectToWiFi(self) -> bool:
        if self.ssid is None or self.password is None:
            self.uart.write("[ERROR] WiFi SSID or Password is empty.")
            return False
        try:
            self.wlan.active(True)
            if not self.wlan.isconnected():
                self.wlan.connect(self.ssid, self.password)
                while not self.wlan.isconnected():
                    self.uart.write(".")
                    sleep(0.5)
            if self.wlan.isconnected():
                self.uart.write("[SUCCESS] Successfully connected to Wifi.")
                self.local_ip = self.wlan.ifconfig()[0]
                return True
        except Exception as e:
            print(f"Error: {e}")
            self.uart.write("[ERROR] Failed to connect to Wifi.")
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
                self.uart.write("[ERROR] SSID or Password is empty.")
                return False

            # Load existing settings
            try:
                with open("flipper-http.json", "r") as f:
                    settings = ujson.loads(f.read())
            except OSError as e:
                if e.errno == errno.ENOENT:
                    settings = {"wifi_list": []}  # Initialize if file doesn't exist
                else:
                    print(e)
                    return False
            except Exception as e:
                self.uart.write(f"[ERROR] Failed to load existing settings: {e}")
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
                    with open("flipper-http.json", "w") as f:
                        f.write(ujson.dumps(settings))
                    self.uart.write("[SUCCESS] Settings saved.")
                    self.ssid = new_ssid
                    self.password = new_password
                    return True
                except Exception as e:
                    self.uart.write(f"[ERROR] Failed to open file for writing: {e}")
                    return False

            self.ssid = new_ssid
            self.password = new_password
            self.uart.write("[INFO] SSID already exists in settings.")
            return True

        except ValueError:
            self.uart.write("[ERROR] Failed to parse JSON data.")
            return False

    def loadWifiSettings(self) -> bool:
        try:
            # Open the settings file and read content
            with open("flipper-http.json", "r") as f:
                file_content = f.read()

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
            print(f"[ERROR] Failed to load or parse settings file: {e}\n")
            return False

    def get(self, url, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        return requests.get(url=url, headers=headers)

    def post(self, url, data, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if isinstance(data, (str, bytes)):
            return requests.post(url, headers=headers, data=data)
        return requests.post(url, headers=headers, data=ujson.dumps(data))

    def put(self, url, data, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if isinstance(data, (str, bytes)):
            return requests.put(url, headers=headers, data=data)
        return requests.put(url, headers=headers, data=ujson.dumps(data))

    def delete(self, url, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        return requests.delete(url, headers=headers)

    def head(self, url, data, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if isinstance(data, (str, bytes)):
            return requests.head(url, headers=headers, data=data)
        return requests.head(url, headers=headers, data=ujson.dumps(data))

    def patch(self, url, data, headers=None) -> Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if isinstance(data, (str, bytes)):
            return requests.patch(url, headers=headers, data=data)
        return requests.patch(url, headers=headers, data=ujson.dumps(data))

    def loop(self):
        """Main loop to handle the serial communication"""
        while True:
            if self.uart.any():
                data = self.uart.read().decode()
                self.ledStatus()

                if data.startswith("[LIST]"):
                    self.uart.write(
                        "[LIST],[PING], [REBOOT], [WIFI/IP], [WIFI/SCAN], [WIFI/SAVE], [WIFI/CONNECT], [WIFI/DISCONNECT], [WIFI/LIST], [GET], [GET/HTTP], [POST/HTTP], [PUT/HTTP], [DELETE/HTTP], [GET/BYTES], [POST/BYTES], [PARSE], [PARSE/ARRAY], [LED/ON], [LED/OFF], [IP/ADDRESS]"
                    )

                # handle [LED/ON] command
                elif data.startsWith("[LED/ON]"):
                    self.use_led = True

                # handle [LED/OFF] command
                elif data.startsWith("[LED/OFF]"):
                    self.use_led = False

                # handle [IP/ADDRESS] command (local IP)
                elif data.startsWith("[IP/ADDRESS]"):
                    self.uart.write(self.local_ip)

                # handle [WIFI/IP] command (wifi IP)
                elif data.startsWith("[WIFI/IP]"):
                    res = self.get("https://ipwhois.app/json/")
                    if res is not None:
                        self.wifi_ip = res.json()["ip"]
                        self.uart.write(self.wifi_ip)
                    else:
                        self.uart.write(
                            "[ERROR] GET request failed or returned empty data."
                        )

                # Ping/Pong to see if board/flipper is connected
                elif data.startsWith("[PING]"):
                    self.uart.write("[PONG]")

                # Handle [REBOOT] command
                elif data.startsWith("[REBOOT]"):
                    # machine.reset()
                    pass

                # scan for wifi networks
                elif data.startsWith("[WIFI/SCAN]"):
                    networks = self.wlan.scan()
                    self.uart.write(ujson.dumps(networks))

                # Handle [WIFI/CONNECT] command
                elif data.startsWith("[WIFI/CONNECT]"):
                    if not self.isConnectedToWiFi():
                        self.connectToWiFi()
                        if self.isConnectedToWiFi():
                            self.uart.write("[SUCCESS] Connected to WiFi")
                        else:
                            self.uart.write("[ERROR] Failed to connect to Wifi.")
                    else:
                        self.uart.write("[INFO] Already connected to WiFi")

                # Handle [WIFI/DISCONNECT] command
                elif data.startsWith("[WIFI/DISCONNECT]"):
                    self.wlan.disconnect()
                    self.uart.write("[DISCONNECTED] Wifi has been disconnected.")

                # Handle [GET] command
                elif data.startsWith("[GET]"):
                    # Extract URL by removing the command part
                    url = data.replace("[GET]", "")
                    res = self.get(url)
                    if res is not None:
                        self.uart.write("[GET/SUCCESS] GET request successful.")
                        self.uart.write(res.text)
                        self.uart.write("[GET/END]")
                    else:
                        self.uart.write(
                            "[ERROR] GET request failed or returned empty data."
                        )

                # Handle [GET/HTTP] command
                elif data.startsWith("[GET/HTTP]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[GET/HTTP]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    if not url:
                        self.uart.write("[ERROR] JSON does not contain url.")
                    if not headers:
                        headers = None
                    res = self.get(data["url"], data["headers"])
                    if res is not None:
                        self.uart.write("[GET/SUCCESS] GET request successful.")
                        self.uart.write(res.text)
                        self.uart.write("[GET/END]")
                    else:
                        self.uart.write(
                            "[ERROR] GET request failed or returned empty data."
                        )

                # Handle [POST/HTTP] command
                elif data.startsWith("[POST/HTTP]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[POST/HTTP]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    payload = data["payload"]
                    if not url or not payload:
                        self.uart.write("[ERROR] JSON does not contain url or payload.")
                    if not headers:
                        headers = None
                    res = self.post(url, payload, headers)
                    if res is not None:
                        self.uart.write("[POST/SUCCESS] POST request successful.")
                        self.uart.write(res.text)
                        self.uart.write("[POST/END]")
                    else:
                        self.uart.write(
                            "[ERROR] POST request failed or returned empty data."
                        )

                # Handle [PUT/HTTP] command
                elif data.startsWith("[PUT/HTTP]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[PUT/HTTP]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    payload = data["payload"]
                    if not url or not payload:
                        self.uart.write("[ERROR] JSON does not contain url or payload.")
                    if not headers:
                        headers = None
                    res = self.put(url, payload, headers)
                    if res is not None:
                        self.uart.write("[PUT/SUCCESS] PUT request successful.")
                        self.uart.write(res.text)
                        self.uart.write("[PUT/END]")
                    else:
                        self.uart.write(
                            "[ERROR] PUT request failed or returned empty data."
                        )

                # Handle [DELETE/HTTP] command
                elif data.startsWith("[DELETE/HTTP]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[DELETE/HTTP]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    if not url:
                        self.uart.write("[ERROR] JSON does not contain url.")
                    if not headers:
                        headers = None
                    res = self.delete(url, headers)
                    if res is not None:
                        self.uart.write("[DELETE/SUCCESS] DELETE request successful.")
                        self.uart.write(res.text)
                        self.uart.write("[DELETE/END]")
                    else:
                        self.uart.write(
                            "[ERROR] DELETE request failed or returned empty data."
                        )

                # Handle [GET/BYTES] command
                elif data.startsWith("[GET/BYTES]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[GET/BYTES]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    if not url:
                        self.uart.write("[ERROR] JSON does not contain url.")
                    if not headers:
                        headers = None
                    res = self.get(url, headers)
                    if res is not None:
                        self.uart.write("[GET/SUCCESS] GET request successful.")
                        self.uart.write(res.content)
                        self.uart.write("[GET/END]")
                    else:
                        self.uart.write(
                            "[ERROR] GET request failed or returned empty data."
                        )

                # Handle [POST/BYTES] command
                elif data.startsWith("[POST/BYTES]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[POST/BYTES]", "")
                    data = ujson.loads(json_data)
                    url = data["url"]
                    headers = data["headers"]
                    payload = data["payload"]
                    if not url or not payload:
                        self.uart.write("[ERROR] JSON does not contain url or payload.")
                    if not headers:
                        headers = None
                    res = self.post(url, payload, headers)
                    if res is not None:
                        self.uart.write("[POST/SUCCESS] POST request successful.")
                        self.uart.write(res.content)
                        self.uart.write("[POST/END]")
                    else:
                        self.uart.write(
                            "[ERROR] POST request failed or returned empty data."
                        )

                # Handle [PARSE] command
                elif data.startsWith("[PARSE]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[PARSE]", "")
                    data = ujson.loads(json_data)
                    json = data["json"]
                    key = data["key"]
                    if not json or not key:
                        self.uart.write("[ERROR] JSON does not contain key or json.")
                    res = ujson.loads(json)
                    if res is not None:
                        found_key = res.get(key)
                        if found_key:
                            self.uart.write(found_key)
                        else:
                            self.uart.write("[ERROR] Key not found in JSON.")
                    else:
                        self.uart.write("[ERROR] JSON parsing failed or key not found.")

                # Handle [PARSE/ARRAY] command
                elif data.startsWith("[PARSE/ARRAY]"):
                    # Extract the JSON by removing the command part
                    json_data = data.replace("[PARSE/ARRAY]", "")
                    data = ujson.loads(json_data)
                    json = data["json"]
                    key = data["key"]
                    index = data["index"]
                    if not json or not key or not index:
                        self.uart.write(
                            "[ERROR] JSON does not contain key, index or json."
                        )
                    res = ujson.loads(json)
                    if res is not None:
                        found_key = res.get(key)
                        if found_key:
                            self.uart.write(found_key[index])
                        else:
                            self.uart.write("[ERROR] Key not found in JSON.")
                    else:
                        self.uart.write("[ERROR] JSON parsing failed or key not found.")

                led.off()
