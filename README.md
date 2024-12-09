# FlipperHTTP
An HTTP library for the Flipper Zero, compatible with the WiFi Developer Board, Raspberry Pi, and other ESP32 devices. View the extended documentation here: [https://www.jblanked.com/api/FlipperHTTP/](https://www.jblanked.com/api/FlipperHTTP/).



Star the repository and [follow me](https://www.github.com/jblanked) to stay updated on new releases and upcoming Flipper apps.

Check out this video tutorial showcasing the functionality of all seven applications: https://www.youtube.com/watch?v=xR1ZbpOVTwE

A big thank you to [Talking Sasquatch](https://github.com/skizzophrenic) for supporting this project. Be sure to watch his review here: https://www.youtube.com/watch?v=jYZkdz664Tg



## Apps That Utilize FlipperHTTP
The following apps integrate FlipperHTTP to enhance their functionality:

- **FlipStore** - A marketplace for downloading and managing apps and software on your Flipper Zero: [https://github.com/jblanked/FlipStore](https://github.com/jblanked/FlipStore)
- **FlipSocial** - A social media platform for the Flipper Zero: [https://github.com/jblanked/FlipSocial](https://github.com/jblanked/FlipSocial)
- **FlipLibrary** - A Flipper Zero app with a dictionary, random facts, and more: [https://github.com/jblanked/FlipLibrary](https://github.com/jblanked/FlipLibrary)
- **Web Crawler** - Allows your Flipper Zero to crawl and interact with websites directly: [https://github.com/jblanked/WebCrawler-FlipperZero](https://github.com/jblanked/WebCrawler-FlipperZero)
- **FlipWeather** - Uses WiFi to fetch GPS and weather information on your Flipper Zero: [https://github.com/jblanked/FlipWeather](https://github.com/jblanked/FlipWeather)
- **FlipTrader** - Uses WiFi to fetch stock and currency pair prices on your Flipper Zero: [https://github.com/jblanked/FlipTrader](https://github.com/jblanked/FlipTrader)
- **FlipWiFi** - A FlipperHTTP companion app for scanning and saving WiFi networks on your Flipper Zero: [https://github.com/jblanked/FlipWiFi](https://github.com/jblanked/FlipWiFi)

All seven applications are available for download at the following link: https://www.flipsocial.net/download/ (or by clicking on the repository link above).



## Wiring
ESP32 -> Flipper
- TX -> Pin 13 (TX)
- RX -> Pin 14 (RX)
- 3v3 -> Pin 9 (3v3)
- GND -> Pin 11 (GND)

Raspberry Pi Pico W/Pico 2 W -> Flipper
- TX (GPIO 0 - Pin 1) -> Pin 13 (TX)
- RX (GPIO 1 - Pin 2) -> Pin 14 (RX)
- 3v3 (VSYS - Pin 39) -> Pin 9 (3v3)
- GND (Pin 38) -> Pin 11 (GND)
- SWCLK (left debug pin) -> Pin 10 (SWC) - This is optional 
- SWDIO (right debug pin) -> Pin 12 (SIO) - This is optional

Raspberry Pi Zero 2 W -> Flipper
- TX (GPIO 14 - Pin 8) -> Pin 14 (RX)
- RX (GPIO 15 - Pin 10) -> Pin 13 (TX)
- GND (Pin 6) -> Pin 11 (GND)
- Do NOT connect 3v3 or 5v (power via USB cable instead)


## Installation
The FlipperHTTP firmware supports eight boards. Instructions for each board can be found below:

- **WiFi Developer Board:** [https://github.com/jblanked/FlipperHTTP/tree/main/WiFi%20Developer%20Board%20(ESP32S2)](https://github.com/jblanked/FlipperHTTP/tree/main/WiFi%20Developer%20Board%20(ESP32S2))
- **Raspberry Pi Pico W:** [https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Pico%20W](https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Pico%20W)
- **Raspberry Pi Pico 2 W:** [https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Pico%202%20W](https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Pico%202%20W)
- **Raspberry Pi Zero 2 W:** [https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Zero%202%20W](https://github.com/jblanked/FlipperHTTP/tree/main/Raspberry%20Pi%20Zero%202%20W)
- **ESP32-C6:** [https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-C6](https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-C6)
- **ESP32-Cam:** [https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-Cam](https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-Cam)
- **ESP32-S3:** [https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-S3](https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-S3)
- **ESP32-WROOM:** [https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-WROOM](https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-WROOM)
- **ESP32-WROVER:** [https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-WROVER](https://github.com/jblanked/FlipperHTTP/tree/main/ESP32-WROVER)

## Usage
FlipperHTTP can be utilized by developers in Flipper Zero apps and scripts written in C, JavaScript, and MicroPython. For details, visit: [https://github.com/jblanked/FlipperHTTP/tree/main/Flipper%20Zero](https://github.com/jblanked/FlipperHTTP/tree/main/Flipper%20Zero)

## Issues
For common bugs and their solutions, see: [https://github.com/jblanked/FlipperHTTP/blob/main/HELP.md](https://github.com/jblanked/FlipperHTTP/blob/main/HELP.md)
