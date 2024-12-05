# Common Issues
If you encounter any problems flashing your board with the FlipperHTTP firmware or using any of the FlipperHTTP apps, refer to the common issues and solutions below. If your issue is not addressed below, report the issue in the repository's `Issues` section.

## FlipperHTTP Firmware
### Issue: "Please enter bootloader mode" in the ESP Flasher app
**Solutions:**
1. Go back to the main menu and click `Reset Board` followed by `Enter Bootloader`.
2. Perform a cold boot by unplugging your board completely and reconnecting it.
3. Force bootloader mode by holding the BOOT button while connecting the board to your Flipper Zero.
4. After receiving the message, follow these steps:
   - Exit to the main menu of the ESP Flasher app.
   - Disconnect your board and reconnect it.
   - Click `Enter Bootloader`, then return to `Manual Flash` and proceed with the flashing steps.

---

## FlipperHTTP Apps
### Issue: "WiFi Devboard is disconnected"
**Solutions:**
1. Check your WiFi settings in the app and re-save them.
2. Restart the app.
3. Exit the app, disconnect the board, reconnect it, and try using the app again.
4. If the above solutions don’t resolve the issue:
   - Open the `UART Terminal` app and click `Open Port`. Change the Baud Rate to `115200`.
   - Look for error messages like `wrong chip` or `load 0x400..`.
   - If such messages appear, the installed firmware is incompatible with your board. 
   - If you sure you installed the correct firmware then report the issue in the repository's `Issues` section. Make sure to include every step you followed (leave no details out).

### Issue: "Failed to receive data" in every app
**Solution:**
1. Update your WiFi credentials in the `Settings` menu option of each app. In the `FlipWiFi` app, this is done in the `Saved APs` option. Verify that you are connecting to a 2.4 GHz network. Supported boards do not support 5 GHz networks. 
