## ESP32-C6 Installation (Windows Only)

1. Download the `flipper_http_bootloader_esp32_c6.bin`, `flipper_http_firmware_a_esp32_c6.bin`, and `flipper_http_partitions_esp32_c6.bin` files.
2. Install the Flash Download Tool by Espressif: [Flash Download Tool Documentation](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html).
3. Follow the user interface instructions to download the ESP32-C6 Manager: [User Interface Instructions](https://docs.espressif.com/projects/esp-test-tools/en/latest/esp32/production_stage/tools/flash_download_tool.html#user-interface).
4. Click on `...` in the first row and select the `flipper_http_bootloader_esp32_c6.bin` file you downloaded earlier. Then set the `@` address to `0x0`.
5. Click on `...` and select the `flipper_http_partitions_esp32_c6.bin` file you downloaded earlier. Then set the `@` address to `0x8000`.
6. Click on `...` and select the `flipper_http_firmware_a_esp32_c6.bin` file you downloaded earlier. Then set the `@` address to `0x10000`.
7. Change the `SPI Speed` to `80 MHz`, `SPI Mode` to `DIO`, and `BAUD` to `115200`. Then click `Start` and wait for the process to complete.

For more details, see [this issue](https://github.com/jblanked/FlipperHTTP/issues/3#issuecomment-2574077289).

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).
