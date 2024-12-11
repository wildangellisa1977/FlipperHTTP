# Raspberry Pi Zero 2 W Installation (Python)

---

## Table of Contents

1. [Prerequisites](#prerequisites)
2. [Preparing the MicroSD Card with Raspberry Pi Imager](#preparing-the-microsd-card-with-raspberry-pi-imager)
3. [Initial Setup and Booting the Raspberry Pi Zero 2 W](#initial-setup-and-booting-the-raspberry-pi-zero-2-w)
4. [Setting Up SSH](#setting-up-ssh)
5. [Updating the System](#updating-the-system)
6. [Configuring Raspberry Pi Settings](#configuring-raspberry-pi-settings)
7. [Disabling Bluetooth](#disabling-bluetooth)
8. [Installing Python Packages](#installing-python-packages)
9. [Troubleshooting](#troubleshooting)

---

## 1. Prerequisites

Before you begin, ensure you have the following:

- **Raspberry Pi Zero 2 W**: Ensure you have the correct model.
- **MicroSD Card**: At least 8GB capacity is recommended.
- **MicroSD Card Reader**: For connecting the MicroSD card to your computer.
- **MicroUSB Cable**: Preferably data-capable, for connecting the Pi to power and your computer.
- **Computer**: Running Windows, macOS, or Linux with internet access.
- **Internet Connection**: For downloading necessary software and updates.

---

## 2. Preparing the MicroSD Card with Raspberry Pi Imager

### Step 1: Insert the MicroSD Card

1. **Insert** your MicroSD card into the computer using a MicroSD card reader.

### Step 2: Download and Install Raspberry Pi Imager

1. **Download** Raspberry Pi Imager from the [official website](https://www.raspberrypi.com/software/).
2. **Install** the software by following the on-screen instructions for your operating system.

### Step 3: Launch Raspberry Pi Imager

1. **Open** Raspberry Pi Imager after installation.

### Step 4: Choose the OS

1. Click on **`CHOOSE OS`**.
2. Navigate to **`Raspberry Pi OS (other)`**.
3. Select **`Raspberry Pi OS Lite (32-bit)`**.

### Step 5: Choose the Device

1. Click on **`CHOOSE STORAGE`**.
2. Select your inserted **MicroSD card**.

### Step 6: Configure Advanced Settings

1. Click on the **gear icon** or **`EDIT SETTINGS`** to access advanced configuration.

#### a. Configure Wireless LAN

- **Check** the option **`Configure wireless LAN`**.
- **Enter** your **WiFi SSID** and **Password**.
  
  *_Important_: Configuring WiFi at this stage prevents future connectivity issues._

#### b. Enable SSH

- Navigate to the **`SERVICES`** tab.
- **Enable** **`SSH`**.
- **Select** **`Use password authentication`**.

### Step 7: Write the OS to the MicroSD Card

1. After configuring settings, click **`SAVE`**.
2. Confirm by clicking **`YES`** when prompted.
3. **Wait** for the imaging process to complete.
4. **Eject** the MicroSD card safely from your computer.

---

## 3. Initial Setup and Booting the Raspberry Pi Zero 2 W

1. **Insert** the prepared MicroSD card into the **Raspberry Pi Zero 2 W**.
2. **Connect** the MicroUSB cable to the **MicroUSB port closest to the camera port**.
3. **Power up** the Raspberry Pi by connecting it to a power source.

*Wait a few moments for the Raspberry Pi to boot up and connect to your WiFi network.*

---

## 4. Setting Up SSH

SSH allows you to access your Raspberry Pi remotely from your computer's terminal.

### Step 1: Open Terminal

- **Windows**: Use **PowerShell** or **Command Prompt**.
- **macOS/Linux**: Use the **Terminal** application.

### Step 2: Ping the Raspberry Pi

1. Type the following command to check connectivity:

   ```bash
   ping pi@raspberrypi.local
   ```

2. **Expected Result**: You should see responses indicating that the host is reachable.

### Step 3: Troubleshooting Unknown Host

If you receive an `Unknown host` message then either:

#### a. Find the Raspberry Pi's IP Address via Your Router

1. **Access** your router's admin page, typically at [http://10.0.0.1](http://10.0.0.1) or [https://192.168.1.1](https://192.168.1.1).
2. **Log in** with your router credentials.
3. **Navigate** to the **connected devices** or **DHCP clients list** to find the Raspberry Pi's IP address.

#### b. Find the IP Address Using Terminal

1. Open a new terminal window.
2. Type the following command:

   ```bash
   ifconfig
   ```

3. **Look** for an IP address associated with the Raspberry Pi on your network.

### Step 4: SSH into the Raspberry Pi

1. Use the IP address found in the previous step. Replace `raspberrypi.local` with the actual IP if necessary.

   ```bash
   ssh pi@raspberrypi.local
   ```

   *Or*

   ```bash
   ssh pi@<IP_ADDRESS>
   ```

2. **First-Time Connection**:

   - You may be prompted with:

     ```
     Are you sure you want to continue connecting (yes/no/[fingerprint])?
     ```

   - **Type** `yes` and **press Enter**.

3. **Enter Password**:

   - When prompted, enter the default password:

     ```
     raspberry
     ```

   *_Note_: It's highly recommended to change the default password after the first login for security reasons._

4. **Successful Connection**:

   - You should see a prompt similar to:

     ```
     pi@raspberrypi:~ $
     ```

---

## 5. Updating the System

Keeping your system updated ensures you have the latest features and security patches.

### Step 1: Update Package Lists

```bash
sudo apt-get update
```

### Step 2: Upgrade Installed Packages

```bash
sudo apt-get upgrade
```

- **When Prompted**, press `Y` and **Enter** to confirm.

*Note: This process may take some time, especially with a slow internet connection.*

### Step 3: Perform a Full Upgrade

```bash
sudo apt full-upgrade
```

### Step 4: Remove Unnecessary Packages

```bash
sudo apt autoremove
```

---

## 6. Configuring Raspberry Pi Settings

### Step 1: Open Raspberry Pi Configuration Tool

```bash
sudo raspi-config
```

### Step 2: Enable VNC

1. **Navigate** to `Interface Options`.
2. **Select** `VNC`.
3. **Enable** VNC.

### Step 3: Configure Serial Port

1. **Still within** `Interface Options`, select `Serial Port`.
2. **Respond** to prompts as follows:

   - **Login Shell Accessible Over Serial?**: Select `No` (This disables the serial console).
   - **Serial Port Hardware Enabled?**: Select `Yes` (This enables the hardware for communication).

### Step 4: Finish Configuration

1. **Click** `OK`.
2. **Scroll** down and select `Finish`.

*If prompted to reboot, choose to reboot now.*

---

## 7. Disabling Bluetooth

Disabling Bluetooth can free up system resources and is useful if you do not require Bluetooth functionality.

### Step 1: Edit the `config.txt` File

```bash
cd /boot
sudo nano config.txt
```

### Step 2: Add Configuration Lines

- **Scroll** to the bottom of the file.
- **Add** the following lines:

  ```bash
  enable_uart=1
  dtoverlay=disable-bt
  ```

### Step 3: Save and Exit

1. **Press** `CTRL + X` to exit.
2. **Press** `Y` to confirm saving changes.
3. **Press** `Enter` to write the changes.

### Step 4: Reboot the Raspberry Pi

```bash
sudo reboot
```

---

## 8. Installing Python Packages

Install `pip` and necessary Python packages to enable your projects.

### Step 1: Install `pip` for Python 3

```bash
sudo apt install python3-pip
```

### Step 2: Upgrade `pip` and Install Packages

```bash
sudo pip3 install --upgrade requests pywifi pyserial --break-system-packages
```

*Note: The `--break-system-packages` flag is used to bypass system package checks. Use it cautiously.*

---

## 9. Troubleshooting

### SSH Connection Issues

- **Unknown Host**: Ensure the Raspberry Pi is connected to the same network as your computer. Verify the IP address via the router or `ifconfig`.
- **Connection Refused**: Make sure SSH is enabled in the Raspberry Pi configuration. Re-run `sudo raspi-config` if necessary.
- **Password Authentication Failure**: Double-check the default password (`raspberry`) or reset it if you've changed it.

### Network Issues

- **No Internet Connection**: Verify WiFi credentials in the Raspberry Pi Imager settings. Ensure the Raspberry Pi is within range of the WiFi router.

### Package Installation Errors

- **Broken Dependencies**: Run `sudo apt-get install -f` to fix broken dependencies.
- **Permission Denied**: Ensure you are using `sudo` where necessary.

---
