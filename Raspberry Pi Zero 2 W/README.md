# Raspberry Pi Zero 2 W Installation (Python)

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Prepare the MicroSD Card with Raspberry Pi Imager](#prepare-the-microsd-card-with-raspberry-pi-imager)
3. [Initial Setup and Booting](#initial-setup-and-booting)
4. [Set Up SSH](#set-up-ssh)
5. [Update the System](#update-the-system)
6. [Configure Raspberry Pi Settings](#configure-raspberry-pi-settings)
7. [Disable Bluetooth](#disable-bluetooth)
8. [Install Python Packages](#install-python-packages)
9. [Run FlipperHTTP Script](#run-flipperhttp-script)
10. [Troubleshooting](#troubleshooting)

---

## 1. Prerequisites

Before you start, make sure you have:

- **Raspberry Pi Zero 2 W**
- **MicroSD Card** (8GB or larger)
- **MicroSD Card Reader**
- **MicroUSB Cable** (data-capable)
- **Computer** with Windows, macOS, or Linux and internet access

---

## 2. Prepare the MicroSD Card with Raspberry Pi Imager

### Step 1: Insert the MicroSD Card

1. Insert the MicroSD card into your computer using the card reader.

### Step 2: Download and Install Raspberry Pi Imager

1. Download Raspberry Pi Imager from the [official website](https://www.raspberrypi.com/software/).
2. Install the software by following the on-screen instructions for your OS.

### Step 3: Launch Raspberry Pi Imager

1. Open Raspberry Pi Imager after installation.

### Step 4: Choose the OS

1. Click **`CHOOSE OS`**.
2. Go to **`Raspberry Pi OS (other)`**.
3. Select **`Raspberry Pi OS Lite (32-bit)`**.

### Step 5: Select the Device

1. Click **`CHOOSE STORAGE`**.
2. Select your MicroSD card.

### Step 6: Configure Advanced Settings

1. Click the **gear icon** or **`EDIT SETTINGS`**.

#### a. Configure Wireless LAN

- Check **`Configure wireless LAN`**.
- Enter your **WiFi SSID** and **Password**.

*Configuring WiFi now prevents future connectivity issues.*

#### b. Enable SSH

1. Go to the **`SERVICES`** tab.
2. Enable **`SSH`**.
3. Choose **`Use password authentication`**.

### Step 7: Write the OS to the MicroSD Card

1. Click **`SAVE`**.
2. Confirm by clicking **`YES`**.
3. Wait for the process to finish.
4. Safely eject the MicroSD card from your computer.

---

## 3. Initial Setup and Booting

1. Insert the prepared MicroSD card into the **Raspberry Pi Zero 2 W**.
2. Connect the MicroUSB cable to the **port closest to the camera**.
3. Power up the Raspberry Pi by connecting it to a power source.

*Wait a few moments for the Pi to boot and connect to your WiFi network.*

---

## 4. Set Up SSH

SSH lets you access your Raspberry Pi remotely.

### Step 1: Open Terminal

- **Windows**: Use **PowerShell** or **Command Prompt**.
- **macOS/Linux**: Open the **Terminal**.

### Step 2: Ping the Raspberry Pi

1. Ensure your computer is on the same network as the Pi.
2. Type the following command to check connectivity:

   ```bash
   ping pi@raspberrypi.local
   ```

3. **Expected**: Responses showing the host is reachable.

### Step 3: If "Unknown Host" Appears

#### a. Find the Pi's IP Address via Router

1. Access your router's admin page (e.g., [http://10.0.0.1](http://10.0.0.1) or [https://192.168.1.1](https://192.168.1.1)).
2. Log in with your router credentials.
3. Check the **connected devices** or **DHCP clients list** for the Pi's IP address.

#### b. Find the IP Address Using Terminal

1. Open a new terminal window.
2. Type:

   ```bash
   ifconfig
   ```

3. Look for the Pi's IP address on your network.

### Step 4: SSH into the Raspberry Pi

1. Use the IP address found earlier. Replace `raspberrypi.local` if needed.

   ```bash
   ssh pi@raspberrypi.local
   ```

   *Or*

   ```bash
   ssh pi@<IP_ADDRESS>
   ```

2. **First-Time Connection**:
   - When prompted, type `yes` and press **Enter**.

3. **Enter Password**:
   - Default password: `raspberry`

   *Change the default password after first login for security.*

4. **Successful Connection**:
   - You should see a prompt like:

     ```
     pi@raspberrypi:~ $
     ```

---

## 5. Update the System

Keeping your system updated ensures security and new features.

### Step 1: Update Package Lists

```bash
sudo apt-get update
```

### Step 2: Upgrade Installed Packages

```bash
sudo apt-get upgrade
```

- Press `Y` and **Enter** when prompted.

*This may take some time.*

### Step 3: Perform a Full Upgrade

```bash
sudo apt full-upgrade
```

### Step 4: Remove Unnecessary Packages

```bash
sudo apt autoremove
```

---

## 6. Configure Raspberry Pi Settings

### Step 1: Open Configuration Tool

```bash
sudo raspi-config
```

### Step 2: Enable VNC

1. Select **`Interface Options`**.
2. Choose **`VNC`**.
3. Enable VNC.

### Step 3: Configure Serial Port

1. Still in **`Interface Options`**, select **`Serial Port`**.
2. Respond to prompts:
   - **Login Shell Accessible Over Serial?**: Select `No`.
   - **Serial Port Hardware Enabled?**: Select `Yes`.

### Step 4: Finish Configuration

1. Click **`OK`**.
2. Select **`Finish`**.
3. If prompted, choose to reboot now.

---

## 7. Disable Bluetooth

Disabling Bluetooth frees up system resources.

### Step 1: Edit the `config.txt` File

```bash
cd /boot
sudo nano config.txt
```

### Step 2: Add Configuration Lines

- Scroll to the bottom and add:

  ```bash
  enable_uart=1
  dtoverlay=disable-bt
  ```

### Step 3: Save and Exit

1. Press `CTRL + X` to exit.
2. Press `Y` to save changes.
3. Press **Enter** to confirm.

### Step 4: Reboot the Raspberry Pi

```bash
sudo reboot
```

---

## 8. Install Python Packages

Install `pip` and necessary Python packages for your projects.

### Step 1: Install `pip` for Python 3

```bash
sudo apt install python3-pip git
```

### Step 2: Upgrade `pip` and Install Packages

```bash
sudo pip3 install --upgrade requests pywifi pyserial --break-system-packages
```

*The `--break-system-packages` flag bypasses system package checks. Use it cautiously.*

---

## 9. Run FlipperHTTP Script

Execute the following commands to get FlipperHTTP files and run the script:

```bash
git clone https://github.com/jblanked/FlipperHTTP.git
cd FlipperHTTP
sudo mv 'Raspberry Pi Zero 2 W' /home/pi
cd /home/pi
sudo mv FlipperHTTPPi2W.py main.py /home/pi
cd /home/pi
sudo rm -r FlipperHTTP 'Raspberry Pi Zero 2 W'
python main.py
```

---

## 10. Troubleshooting

### SSH Connection Issues

- **Unknown Host**: Ensure the Pi is on the same network. Verify the IP via router or `ifconfig`.
- **Connection Refused**: Make sure SSH is enabled. Re-run `sudo raspi-config` if needed.
- **Password Authentication Failed**: Check the password (`raspberry`) or reset it if changed.

### Network Issues

- **No Internet Connection**: Verify WiFi credentials in Raspberry Pi Imager. Ensure the Pi is within WiFi range.

### Package Installation Errors

- **Broken Dependencies**: Run `sudo apt-get install -f` to fix.
- **Permission Denied**: Use `sudo` where necessary.


For wiring information, click [here](https://github.com/jblanked/FlipperHTTP?tab=readme-ov-file#wiring).