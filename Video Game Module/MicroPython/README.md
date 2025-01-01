## Video Game Module Installation (MicroPython)

1. Download the `main.py` and `FlipperHTTPVGM.py` files.
2. Press and hold the `BOOT` button on your Video Game Module for 2 seconds.
3. While holding the `BOOT` button, connect the Video Game Module to your computer using a USB cable.
4. The Video Game Module should appear as a storage device on your computer.
5. Download Thonny from [https://thonny.org](https://thonny.org).
6. Install MicroPython:
   - Go to `Run -> Configure interpreter`.
   - Select `MicroPython (Raspberry Pi Pico)` in the `Which kind of interpreter...` field.
   - Click `Install or update MicroPython`. The target volume and MicroPython family should appear. If not, ensure your Video Game Module is connected.
   - Choose `Raspberry Pi - Pico` in the `Variant` field.
   - Click `Install`. When the installation is complete, your Video Game Module will automatically reboot and start running MicroPython.
7. Click `OK` to close the dialog and return to the main window.
8. Add the Files:
    - Click `Load`, then `This computer`, and select the `FlipperHTTPVGM.py` file you downloaded earlier.
    - Go to the `File` menu, select `Save copy`, then choose `Raspberry Pi Pico`. Name the file `FlipperHTTPVGM.py` and click `OK`.
    - Repeat the steps above for the `main.py`, saving it to the Video Game Module with the name `main.py`.

You're all set! When you reboot your Video Game Module, it will run the FlipperHTTP Flash. 