## BW16 Installation
1. Install the Arduino IDE from https://www.arduino.cc/en/software
2. Download this repository as a .zip file (available here: https://github.com/jblanked/FlipperHTTP/archive/refs/heads/main.zip)
3. Unzip the file and navigate to the `src` folder. Double-click the `flipper-http.ino` file to open it in Arduino IDE.
4. In Arduino IDE, go to Preferences/Settings and add the following URL to the `Additional Boards Manager URLs` field: `https://github.com/Ameba-AIoT/ameba-arduino-d/raw/dev/Arduino_package/package_realtek_amebad_early_index.json`. Click `OK`.
5. In `Boards Manager`, search for `realtek` and install the `Realtek Ameba Boards (32-bit ARM Cortex)` package. 
6. In `Libraries`, search for `FlashStorage` and install the `FlashStorage_RTL8720` library by `Khoi Hoang`.
7. Restart your Arduino IDE.
8. Hold the `BURN` button while connecting your USB-C data cable.
9. Select the serial port to which your BW16 is connected and choose `Ai-Thinker BW16 (RTL8720DN)` as your board.
10. Click `Sketch` in the menu, then select `Upload`.
11. After the upload completes, press the `RST` button once. If your board is flashed correctly, the BW16's LED will flash GREEN three times.

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).
