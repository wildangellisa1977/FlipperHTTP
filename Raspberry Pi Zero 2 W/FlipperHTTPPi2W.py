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
import errno
import requests
import serial


class FlipperHTTP:
    def __init__(self):
        self.local_ip = None
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
            bytesize=serial.EIGHTBITS,
        )
        self.use_led = True
        self.flush()
        self.clearSerialBuffer()

    def clearSerialBuffer(self) -> None:
        """Clear the serial buffer"""
        self.uart.reset_input_buffer()  # Clear the input buffer
        self.uart.reset_output_buffer()  # Clear the output buffer

    def get(self, url, headers=None) -> requests.Response:
        return requests.get(url, headers=headers)

    def post(self, url, payload, headers=None) -> requests.Response:
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.post(url, headers=headers, data=payload)
        return requests.post(url, headers=headers, json_data=json.dumps(payload))

    def put(self, url, payload, headers=None) -> requests.Response:
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.put(url, headers=headers, data=payload)
        return requests.put(url, headers=headers, json_data=json.dumps(payload))

    def delete(self, url, headers=None) -> requests.Response:
        return requests.delete(url, headers=headers)

    def head(self, url, payload, headers=None) -> requests.Response:
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.head(url, headers=headers, data=payload)
        return requests.head(url, headers=headers, json_data=json.dumps(payload))

    def patch(self, url, payload, headers=None) -> requests.Response:
        if payload is None:
            return None
        if isinstance(payload, (str, bytes)):
            return requests.patch(url, headers=headers, data=payload)
        return requests.patch(url, headers=headers, json_data=json.dumps(payload))

    def write(self, message: bytes):
        self.uart.write(message)

    def println(self, message: str) -> bool:
        encoded_message = (message + "\n").encode("utf-8")
        bytes_sent = self.uart.write(encoded_message)
        is_sent = bytes_sent == len(encoded_message)
        if not is_sent:
            self.saveError("Failed to send message.")
            print("Failed to send message.")
        return is_sent

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
                uart_data = self.uart.readline()
                if uart_data is None:
                    continue
                else:

                    if uart_data is None or uart_data == b"":
                        continue
                    data = uart_data.decode("utf-8").strip()

                    if "\n" in data:
                        data = data.strip("\n")

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
