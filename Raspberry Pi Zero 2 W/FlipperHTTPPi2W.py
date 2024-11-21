""" FlipperHTTP-Pi2w.py (for the Raspberry Pi 2W)
Author: JBlanked
Github: https://github.com/jblanked/FlipperHTTP
Info: This library is a wrapper around the HTTPClient library and is used to communicate with the FlipperZero over serial.
Created: 2024-11-20
Updated: 2024-11-20

Change Log:
- 2024-11-20: Initial commit
"""

import json
import time
from time import sleep
import errno
import requests
import serial
import pywifi  # pip install pywifi (only for Linux/Windows)
from pywifi import const


class FlipperHTTP:
    def __init__(self):
        self.local_ip = None
        self.wlan = pywifi.PyWiFi().interfaces()[0]
        self.wifi_ip = None
        self.ssid = None
        self.password = None
        self.use_led = True
        self.timeout = 2000  # milliseconds
        self.uart = None
        self.BAUD_RATE = 115200

    def saveError(self, err, is_os_error: bool = False) -> None:
        if not is_os_error:
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

            with open("error.txt", "w") as f:
                f.write(f"OSError: {reason}")

    def setup(self) -> None:
        """Start UART and load the WiFi credentials"""
        self.uart = serial.Serial(
            "/dev/serial0",
            baudrate=self.BAUD_RATE,
            timeout=(self.timeout / 1000),
            parity=serial.PARITY_NONE,
            stopbits=serial.STOPBITS_ONE,
            bytesize=serial.SEVENBITS,
        )
        self.use_led = True
        self.loadWifiSettings()
        self.flush()
        self.clearSerialBuffer()

    def clearSerialBuffer(self) -> None:
        """Clear the serial buffer"""
        self.uart.reset_input_buffer()  # Clear the input buffer
        self.uart.reset_output_buffer()  # Clear the output buffer

    def connectToWiFi(self) -> bool:
        if self.ssid is None or self.password is None:
            self.println("[ERROR] WiFi SSID or Password is empty.")
            return False
        try:
            if self.wlan.status() in [const.IFACE_DISCONNECTED, const.IFACE_INACTIVE]:
                profile = pywifi.Profile()
                profile.ssid = self.ssid
                profile.auth = pywifi.const.AUTH_ALG_OPEN
                profile.akm.append(pywifi.const.AKM_TYPE_WPA2PSK)
                profile.cipher = pywifi.const.CIPHER_TYPE_CCMP
                profile.key = self.password
                self.wlan.connect(profile)

                while not self.wlan.status() == const.IFACE_CONNECTED:
                    self.uart.write(".")
                    sleep(0.5)
            if self.wlan.status() == const.IFACE_CONNECTED:
                self.println("[SUCCESS] Successfully connected to Wifi.")
                self.local_ip = self.wlan.ifconfig()[0]
                return True
        except Exception as e:
            self.saveError(e)
            self.println("[ERROR] Failed to connect to Wifi.")
            return False

    def isConnectedToWiFi(self) -> bool:
        return self.wlan.status() == const.IFACE_CONNECTED

    def saveWifiSettings(self, jsonData: str) -> bool:
        try:
            # Parse JSON data
            data = json.loads(jsonData)
            new_ssid = data.get("ssid")
            new_password = data.get("password")

            if not new_ssid or not new_password:
                self.println("[ERROR] SSID or Password is empty.")
                return False

            # Load existing settings
            try:
                with open("flipper-http.json", "r") as f:
                    settings = json.loads(f.read())
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
                    with open("flipper-http.json", "w") as f:
                        f.write(json.dumps(settings))
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
            with open("flipper-http.json", "r") as f:
                file_content = f.read()

            if not file_content:
                return False

            # Parse JSON content
            settings = json.loads(file_content)
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

    def get(self, url, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        return requests.get(url, headers=headers)

    def post(self, url, payload, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.post(url, headers=headers, data=payload)
        return requests.post(url, headers=headers, json_data=json.dumps(payload))

    def put(self, url, payload, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.put(url, headers=headers, data=payload)
        return requests.put(url, headers=headers, json_data=json.dumps(payload))

    def delete(self, url, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        return requests.delete(url, headers=headers)

    def head(self, url, payload, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.head(url, headers=headers, data=payload)
        return requests.head(url, headers=headers, json_data=json.dumps(payload))

    def patch(self, url, payload, headers=None) -> requests.Response:
        if not self.isConnectedToWiFi() and not self.connectToWiFi():
            return None
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.patch(url, headers=headers, data=payload)
        return requests.patch(url, headers=headers, json_data=json.dumps(payload))

    def write(self, message: bytes):
        self.uart.write(message)

    def println(self, message: str):
        self.uart.write((message + "\n").encode("utf-8"))

    def flush(self):
        self.uart.flush()

    def readSerialLine(self) -> str:
        data = ""
        try:
            raw_data = self.uart.read()
            if raw_data:  # Ensures raw_data isn't empty before decoding
                data = raw_data.decode("utf-8")
        except Exception as e:
            pass  # raw_data is empty/None
        return data

    def readLine(self) -> str:
        start_time = int(time.time_ns() / 1000000)
        message = ""

        while (int(time.time_ns() / 1000000) - start_time) < self.timeout:
            try:
                raw_data = self.uart.read()
                if raw_data:  # Check if data was actually read
                    # Reset the timeout when data is read
                    start_time = int(time.time_ns() / 1000000)
                    message += raw_data.decode("utf-8")  # Decode bytes to string

                    if "\n" in message:  # Stop reading when a newline is encountered
                        message = message.strip("\n")
                        return message
            except Exception as e:
                continue

        # Timeout reached with no newline received
        return None

    def loop(self):
        """Main loop to handle the serial communication"""
        while True:
            # gc.collect()  # Run garbage collection to free up memory

            try:
                uart_data = self.uart.read()
                if uart_data is None:
                    continue
                else:
                    data = self.uart.readline()
                    if data is None or data == b"":
                        continue
                    data = data.decode("utf-8").strip()

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
                        # machine.reset()
                        pass

                    # scan for wifi networks
                    elif data.startswith("[WIFI/SCAN]"):
                        try:
                            self.wlan.scan()
                            networks = self.wlan.scan_results()
                            network_data = []
                            # list of SSIds only
                            for network in networks:
                                network_data.append(network.ssid)
                            self.println(json.dumps(network_data))
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
                        data = json.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        if not url:
                            self.println("[ERROR] JSON does not contain url.")
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
                        data = json.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        payload = data["payload"]
                        if not url or not payload:
                            self.println(
                                "[ERROR] JSON does not contain url or payload."
                            )
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
                        data = json.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        payload = data["payload"]
                        if not url or not payload:
                            self.println(
                                "[ERROR] JSON does not contain url or payload."
                            )
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
                        data = json.loads(json_data)
                        url = data["url"]
                        headers = data["headers"]
                        if not url:
                            self.println("[ERROR] JSON does not contain url.")
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
                            data = json.loads(json_data)

                            url = data["url"]
                            headers = data.get("headers", None)

                            if not url:
                                self.println("[ERROR] JSON does not contain url.")

                                return  # Exit the handler if URL is missing

                            # Initialize the response variable
                            res = None

                            # Make the GET request
                            gc.collect()
                            try:
                                res = self.get(url, headers)
                            except Exception as e:
                                self.saveError(e)
                                return

                            # stream the response
                            if res is not None:
                                self.println("[GET/SUCCESS] GET request successful.")
                                for chunk in res.iter_content(chunk_size=2048):
                                    self.uart.write(chunk)
                                self.println("[GET/END]")
                                self.flush()

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
                            data = json.loads(json_data)

                            url = data["url"]
                            headers = data.get("headers", None)
                            payload = data["payload"]

                            if not url or not payload:
                                self.println(
                                    "[ERROR] JSON does not contain url or payload."
                                )
                                return  # Exit the handler if URL is missing

                            # Initialize the response variable
                            res = None

                            # Make the POST request
                            gc.collect()
                            try:
                                res = self.post(url, payload, headers)
                            except Exception as e:
                                self.saveError(e)
                                return

                            # stream the response
                            if res is not None:
                                self.println("[POST/SUCCESS] POST request successful.")
                                for chunk in res.iter_content(chunk_size=2048):
                                    self.uart.write(chunk)
                                self.println("[POST/END]")
                                self.flush()

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
                        data = json.loads(json_data)
                        json_edit = data["json"]
                        key = data["key"]
                        if not json_edit or not key:
                            self.println("[ERROR] JSON does not contain key or json.")
                            return
                        res = json.loads(json_edit)
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
                        data = json.loads(json_data)
                        json_edit = data["json"]
                        key = data["key"]
                        index = data["index"]
                        if not json_edit or not key or not index:
                            self.println(
                                "[ERROR] JSON does not contain key, index or json."
                            )
                            return
                        res = json.loads(json_edit)
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
            except KeyboardInterrupt:
                self.uart.close()
                break
            except Exception as e:
                self.saveError(e)
                self.println(f"[ERROR] {e}")
                continue
