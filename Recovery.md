# How to recover from a corrupted ESP firmware (DCHDMI is dead)

If your DCHDMI is dead after a firmware upgrade from the OSD, it's likely that the ESP firmware is corrupted. There was a bug in the OSD upgrade, which is fixed in v2.2.0, so it can happen in any firmware before that. The issue was caused by a missing checksum verification, when flashing a new firmware.

Firmwares after v1.2.0 should at least give you a hint, that something went wrong during firmware download, by showing an error for the specific part of the firmware: 

```
[ ERROR DOWNLOADING  ] done.
```

But this didn't prevent you from flashing the corrupted file leading to a bricked device.

### How to recover

#### Preparation

- You will need [either Python 2.7 or Python 3.4 or newer](https://www.python.org/downloads/) installed on your system.

- [`esptool.py`](https://github.com/espressif/esptool)

  The latest stable esptool.py release can be installed from pypi via pip:
  ```
  pip install esptool
  ```

- A 3.3V serial port adapter.

  - [SparkFun Beefy 3 - FTDI Basic Breakout](https://www.sparkfun.com/products/13746) will fit directly to the DCHDMI serial port using a 2.54mm header and is able to supply 3.3V to DCHDMI while flashing.

    You don't have to solder the 2.54mm header to the board, just plug it into the serial port adapter and hold it at an angle applying light pressure, so it makes good contact to the holes on the DCHDMI board.

    Be sure the markings on the adpater line up with the markings on the DCHDMI board. (the adapter side with the USB port is facing away from the DCHDMI board)

    *No external power supply is needed, when using this serial port adapter.*

  - [FT232RL 3.3V 5.5V FTDI USB to TTL Serial Adapter Module](https://www.amazon.com/XCSOURCE-FT232RL-Adapter-Arduino-TE203/dp/B00HSX3CXE/)

    If you are using this adapter make sure the jumper is set to **3.3V** and the adapter side with the USB port is facing towards the DCHDMI board.

    *This might also work without using an external power supply, but it will exceed the 50mA current the FT232 can provide. The FT232 IC will get hot, when you're powering DCHDMI this way, and the serial port adapter might get damaged.*

  - If using your own serial port adapter make sure it's working at 3.3V! Otherwise DCHDMI might get damaged. (It's not 5V tolerant!)

    Also make sure, that your adapter is capable of supplying 200mA@3.3V or you have to use an external power supply.

    Only `3.3V`, `GND`, `RX` and `TX` have to be connected.

- Download the ESP firmware and filesystem from [esp.i74.de](https://esp.i74.de/master/)

  You will need the following files:

  - [4MB-firmware.bin](https://esp.i74.de/master/4MB-firmware.bin)
  - [4MB-spiffs.bin](https://esp.i74.de/master/4MB-spiffs.bin)

- Remove the DCHDMI mainboard from you Dreamcast.

#### Procedure

- Set switch 2 of SW1 on DCHDMI to `on` (as shown below). Leave switch 1 as it is. 

  This sets the ESP to "ROM serial bootloader" mode, where it can be programmed using `esptool.py`

  <img src="assets/switch.jpg" width="70%"/>

- Connect serial port adapter.

- Use the following command to flash the ESP.

  *Be sure, that you execute the command in the folder, where you previously saved the files.*
  ```
  esptool.py -p <COM_PORT> write_flash 0x00000000 4MB-firmware.bin 0x00100000 4MB-spiffs.bin
  ```
  `COM_PORT` should be something like `COM6` (on Windows) or `/dev/tty.usbserial-A50285BI` (on Linux/Mac OS X)

  You should see this output:
  ```
  esptool.py v2.3.1
  Connecting....
  Detecting chip type... ESP8266
  Chip is ESP8266EX
  Features: WiFi
  Uploading stub...
  Running stub...
  Stub running...
  Configuring flash size...
  Auto-detected Flash size: 4MB
  Compressed 425152 bytes to 286193...
  Wrote 425152 bytes (286193 compressed) at 0x00000000 in 25.3 seconds (effective 134.2 kbit/s)...
  Hash of data verified.
  Compressed 3125248 bytes to 257676...
  Wrote 3125248 bytes (257676 compressed) at 0x00100000 in 22.8 seconds (effective 1095.4 kbit/s)...
  Hash of data verified.
  
  Leaving...
  Hard resetting via RTS pin...
  ```
  
  **ESP firmware is now successfully flashed**

- Disconnect serial port adapter.

- Be sure to set switch 2 of SW1 on DCHDMI to `off`, otherwise the ESP will still boot up in "ROM serial bootloader" mode once your Dreamcast is reassembled.

# How to recover from a restarting v4.1 firmware

There are two ways to recover from the **Console keeps restarting with v4.1** bug:

1) Try start your console and wait if the restarts stop occuring. If it keeps restarting (more than 5 times), try powering the console off and on again. As soon as it manages to get beyond the MDNS configuration stage, it will run without an issue (until next boot). Now you can update the firmware as usual.

2) Use AP mode and web console to manually update the firmware. In AP mode, the MDNS initialization is skipped.

    1) Make sure your dreamcast is forced into AP mode by disconnecting it from your wifi network. Easiest way to achieve this is to change the wifi password of your wifi router, so DCHDMI will fall back to AP mode (this can take up to 30 seconds).

    2) Download the new (ESP) firmware here: https://dc.i74.de/esp/master/4MB-firmware.bin

    3) Open the OSD and enter the `WiFi Setup` page.

    4) Make sure it says `[Access point]` on the screen.

        <img src="https://github.com/chriz2600/DreamcastHDMI/raw/bleeding/assets/DCHDMI-accesspoint.png" alt="WiFi connected" width="50%"/>

    5) Press `Y` to reveal passwords.<br>To be able to connect to the DCHDMI access point you will need the information marked <span style="padding:3px;background-color:black;color:red;">**red**</span>.<br>For web console access you will need the information marked <span style="padding:3px;background-color:black;color:yellow">**yellow**</span>.

        <img src="https://github.com/chriz2600/DreamcastHDMI/raw/bleeding/assets/DCHDMI-accesspoint-markers.png" alt="WiFi connected" width="50%"/>

    6) Connect your WiFi capable computer to the DCHDMI access point using:
        
        - SSID: `Access point SSID`
        
        - Password: `Access point password`

    7) When connected to DCHDMI AP, use a browser of your choice and enter `192.168.4.1` in the browsers address bar.

    8) The *Web console* should be displayed and setup mode should be started. Just hit `CTRL-D` to exit the setup.

    9) Use `select` to select the previously downloaded **ESP firmware** (`4MB-firmware.bin`).

    10) `uploadesp` to upload.

    11) Now use `flashesp` to flash the ESP firmware.

    12) `reset` DCHDMI to (re-)start with the new firmware
