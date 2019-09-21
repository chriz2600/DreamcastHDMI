# ESP8266 companion for DreamcastHDMI / DCHDMI

This was designed to provide an easy way to upgrade the firmware of the DreamcastHDMI/DCHDMI project using an **ESP-07S**, but it should be easily adapted to other projects, where a SPI Flash should be programmed (in a failsafe manner, as the ESP-07S firmware is not altered).

##### Why ESP-07S? Why not use ESP-12e?

The ESP is operating inside the dreamcast's metal shielding, so it is necassary to mount the antenna outside of this. A perfect match for this is the ESP-07S, which comes without an external antenna. It usually also has 4MB instead of 1MB of flash memory.

![ESP-07S on DCHDMI](https://media.githubusercontent.com/media/chriz2600/DreamcastHDMI/bleeding/ESP/misc/esp07s_2.jpg)

## Initial setup:

On first startup the module acts as access point with predefined ssid/password. When connected you can setup the module, so it connects to your wifi router. 

The setup credentials are:

```
SSID: dc-firmware-manager<ID>
PSK:  geheim1234
IP:   192.168.4.1
URL:  http://192.168.4.1
User: Test
Pass: testtest
```

After connecting to [URL](http://192.168.4.1), you will be guided through the setup process.

**Be sure to set the "OTA Password" to be able to upload a newer ESP firmware "over the air" in the future!**

Restart the ESP-07S after setup is done with the command `restart`. If SSID and password are correct the Firmware Manager should now be connected to your local WiFi network.

## Connecting to module after setup:

After that - if your system supports mDNS (MAC OS X, Linux with avahi and Windows with Apple bonjour installed) - you can connect to [dc-firmware-manager.local][dcfwm] to flash a new firmware.

I've also created a small demo, running on a webserver:   
Just type "help" and "details" to get information about how to upgrade firmware.

[dc-fw-manager.i74.de][dcfwdemo]

## To build ESP-07S firmware:

- To build you need platformio:

| Command | Comment |
|-|-|
| `pio run` | to build |
| `pio upload` | to upload to ESP-07S (remember to check upload related settings in platformio.ini) |

## To create inlined index.html:

To build index.html you need the following

- node
- inliner node module (`npm install -g inliner`)

`./local/prepare-index-html` to create compressed, self-contained index.html

## Demo:

- [Demo][dcfwdemo]

## Flashing firmware:

I highly recommend the newer [esptool.py](https://github.com/espressif/esptool) instead of the esptool from platformio, as esptool.py is much faster by using a compressed protocol.

To flash the firmware the first time, you need a serial port (e.g. a usb to serial adapter). Be sure to set it to 3.3V, as the ESP8266 is not 5V tolerant.

The latest firmware is always available on [esp.i74.de](https://esp.i74.de/master/).

The firmware is divided into 2 parts, one (firmware.bin) is the actual firmware, the other (spiffs.bin) is the initial flash file system.

#### First time flash:

To flash the firmware/filesystem image the ESP-07S must be booted into serial programming mode.

See [ESP8266 Boot Mode Selection](https://github.com/espressif/esptool/wiki/ESP8266-Boot-Mode-Selection) for details.

If you have to program more than one ESP [this](https://www.tindie.com/products/petl/esp12-programmer-board-with-pogo-pins/) might come in handy.

```
esptool.py -p <serial_port> write_flash 0x00000000 firmware.bin 0x00100000 spiffs.bin

serial_port: 
    e.g. COM5 on windows, /dev/cu.usbserial-A50285BI on OSX.

# or 
pio run -t upload
pio run -t uploadfs

platformio.ini:
    upload_port = /dev/cu.usbserial-A50285BI
    upload_speed = 230400
```

After that, the ESP can be programmed "over the air", if a OTA password was set in setup before.

To do an OTA update you need [espota.py](https://github.com/esp8266/Arduino/blob/master/tools/espota.py) or platformio

```
espota.py -r -i dc-firmware-manager.local -p 8266 -a <OTA-password> -f firmware.bin

OTA-password:
    this must be set via the `setup` command.

# or
pio run -t upload

platformio.ini:
    upload_port = dc-firmware-manager.local
    upload_flags = --auth=<OTA-password>
```

## DC firmware file format

The firmware manager uses a custom [FastLZ][fastlz] based archive format. The byte order for values is little-endian.

##### Header

The header is 16-bytes long.

| Byte | Description | Value | Notes |
| -:| - |:-:| - |
| `0`<br>`1`<br>`2`<br>`3` | 4 byte file identification | `0x44`<br>`0x43`<br>`0x07`<br>`0x04` | *fixed* |
| `4`<br>`5` | 2 byte version number | `0x0n`<br>`0x00` | *version can be 1 or 2* |
| `6`<br>`7` | 2 byte `block_size` used during compression | `block_size[0]`<br>`block_size[2]` | *default is 1536*<br>`00` `06` |
| `8`<br>`9`<br>`10`<br>`11` | 4 bytes `file_size` of the decompressed file | `file_size[0]`<br>`file_size[1]`<br>`file_size[2]`<br>`file_size[3]` |   |
| `12`<br>`13`<br>`14`<br>`15` | **v1**: 4 bytes reserved<br>**v2**: 1 byte `bundled_archives`, <br>3 bytes reserved | `bundled_archives`(v2)<br>`0x00`<br>`0x00`<br>`0x00` | *v2 supports bundling up to 8 archives into one bundle* |

##### Footer ***(v2 only)***

Version 2 allows bundling multiple archives in one archive bundle. The start position of each archive is stored at the end of the bundle after the last archive.

| Word | Description | Value | Notes |
| - | - |:-:| - |
| `position first archive`<br>`...`<br>`position last archive` | Up to 8 4-byte words | `pos_0[0:3]`<br>`...`<br>`pos_N[0:3]` |   |

##### Payload

The data payload contains the compressed firmware configuration data, which is decompressed and then flashed - in chunks of `256` byte - to the configuration memory/SPI flash.

Each data packet looks like this:

| Byte | Description | Value | Notes |
| - | - |:-:| - |
| `0`<br>`1` | 2 byte `chunk_size` | `chunk_size[0]`<br>`chunk_size[1]` |   |
| `2`<br>...<br>`2`<br>+`chunk_size` | `chunk_size` bytes [FastLZ][fastlz] compressed data, which decompresses to `block_size` bytes | *`data`* |   |

##### Notes

- C language implementation of both packer and unpacker can be found [here](https://github.com/chriz2600/DreamcastHDMI/tree/master/firmware-utils). The packer is used in the automatic build chain, to create firmware files on [dc.i74.de](https://dc.i74.de).

- Decompression/flashing implementation can be found [here](https://github.com/chriz2600/DreamcastHDMI/blob/bleeding/ESP/src/task/FlashTask.h).

- Standard `block_size` is `1536`. Greater `block_size` may increase achievable compression, but keep in mind that the ESP8266 has only 80kB of RAM.

## Schematic:

![Schematic](https://media.githubusercontent.com/media/chriz2600/DreamcastHDMI/bleeding/ESP/misc/DCFirmwareManager.png)

----

[dcfwdemo]: http://dc-fw-manager.i74.de/
[esp07]: https://www.esp8266.com/wiki/doku.php?id=esp8266-module-family#esp-07
[dcfwm]: http://dc-firmware-manager.local
[fastlz]: https://github.com/ariya/FastLZ