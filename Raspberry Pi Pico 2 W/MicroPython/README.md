## Raspberry Pi Pico 2 W Installation (MicroPython)

1. Download the `main.py`, `urequests_2.py`, `EasySD.py`, and `FlipperHTTPPico2W.py` files.
2. Press and hold the `BOOT` button on your Pico W for 2 seconds.
3. While holding the `BOOT` button, connect the Pico W to your computer using a USB cable.
4. The Pico W should appear as a storage device on your computer.
5. Download Thonny from [https://thonny.org](https://thonny.org).
6. Install MicroPython:
   - Go to `Run -> Configure interpreter`.
   - Select `MicroPython (Raspberry Pi Pico)` in the `Which kind of interpreter...` field.
   - Click `Install or update MicroPython`. The target volume and MicroPython family should appear. If not, ensure your Pico W is connected.
   - Choose `Raspberry Pi - Pico 2 W` in the `Variant` field.
   - Click `Install`. When the installation is complete, your Pico W will automatically reboot and start running MicroPython.
7. Click `OK` to close the dialog and return to the main window.
8. Add the Files:
    - Click `Load`, then `This computer`, and select the `FlipperHTTPPico2W.py` file you downloaded earlier.
    - Go to the `File` menu, select `Save copy`, then choose `Raspberry Pi Pico`. Name the file `FlipperHTTPPico2W.py` and click `OK`.
    - Repeat the steps above for `main.py`, `EasySD.py`, and `urequests_2.py`, saving it to the Raspberry Pi Pico with the name `main.py`, `EasySD.py`, and `urequests_2.py`.

You're all set! When you reboot your Pico W, it will run the FlipperHTTP Flash. Here's a video tutorial: https://www.youtube.com/watch?v=rdzKDCjbZ4k
