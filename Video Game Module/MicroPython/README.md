## Video Game Module Installation (MicroPython)

There are two installation methods. Choose whichever is the most convenient for you.

### Method 1
1. Install the Video Game Module Tool app on your Flipper Zero from the Apps catalog: [Video Game Module Tool](https://lab.flipper.net/apps/video_game_module_tool).
2. Download the `flipper_http_vgm_micro_python.uf2` file.
3. Disconnect your Video Game Module from your Flipper and connect your Flipper to your computer.
4. Open qFlipper.
5. Click on the `File manager`.
6. Navigate to `SD Card/apps_data/vgm/`. If the folder doesn’t exist, create it yourself. Inside that folder, create a folder called `FlipperHTTP`.
7. Drag the `flipper_http_vgm_micro_python.uf2` file you downloaded earlier into the directory.
8. Disconnect your Flipper from your computer, then turn off your Flipper.
9. Connect your Video Game Module to the Flipper, then turn on your Flipper.
10. Open the Video Game Module Tool app on your Flipper. It should be located in the `Apps->GPIO` folder from the main menu.
11. In the Video Game Module Tool app, select `Install Firmware from File`, then `apps_data`.
12. Scroll down and click on the `vgm` folder, then `FlipperHTTP`, and then `flipper_http_vgm_micro_python.uf2`.
13. The app will begin flashing the firmware to your Video Game Module. Wait until the process is complete.

### Method 2
1. Download the `flipper_http_vgm_micro_python.uf2` file.
2. Press and hold the `BOOT` button on your Video Game Module for 2 seconds.
3. While holding the `BOOT` button, connect the Video Game Module to your computer using a USB cable.
4. Drag and drop the downloaded file onto the device. It will automatically reboot and begin running the FlipperHTTP firmware.

For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).

**Note:** It's best to use the MicroPython file currently, as there's a bug in the MICRO_PYTHON version. This will be resolved soon, and all contributions are welcome.