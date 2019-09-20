# Web console

[Jump to command reference](#commands)

## How to connect

If your DCHDMI is already connected to your local WiFi network:

1) Open the OSD and go to `WiFi Setup`.

2) Make sure it says `[Connected]` instead of `[Access point]` on the screen.

    <img src="https://github.com/chriz2600/DreamcastHDMI/raw/bleeding/assets/DCHDMI-connected.png" alt="WiFi connected" width="50%"/>


3) Press `Y` to reveal the `Web login password`. You can skip that, if you set an password in the Web console's setup.

4) Use a browser of your choice and enter the IP address shown next to `IP address` in the browsers address bar.

5) A login windows should open. Enter `Web login username` and `Web login password`.

6) A command prompt should be displayed.


## How to update the Firmware from the web console

*This assumes your DCHDMI is connected to a WiFi network.*

1) Connect to the web console.

2) *Optional*: use `check` to see, if a new firmware is available.

3) `download` new firmware

4) `flash` new firmware

5) `reset` DCHDMI to (re-)start with the new firmware.


## I don't want to connect my DCHDMI to my WiFi network, how do I update manually?

*This assumes your DCHDMI is not connected to your local WiFi network. It also assumes you're using v4.0 or newer.*

1) Download the firmware files to your computer:

    a) FPGA firmware: https://dc.i74.de/fw/master/DCxPlus-v2.dc

    b) ESP firmware: https://dc.i74.de/esp/master/4MB-firmware.bin

    c) Web console: https://dc.i74.de/esp/master/esp.index.html.gz

2) Open the OSD and enter the `WiFi Setup` page.

3) Make sure it says `[Access point]` on the screen.

    <img src="https://github.com/chriz2600/DreamcastHDMI/raw/bleeding/assets/DCHDMI-accesspoint.png" alt="WiFi connected" width="50%"/>

4) Press `Y` to reveal passwords.<br>To be able to connect to access point you will need the information marked <span style="padding:3px;background-color:black;color:red;">**red**</span>.<br>For web console access you will need the information marked <span style="padding:3px;background-color:black;color:yellow">**yellow**</span>.

    <img src="https://github.com/chriz2600/DreamcastHDMI/raw/bleeding/assets/DCHDMI-accesspoint-markers.png" alt="WiFi connected" width="50%"/>

5) Connect your WiFi capable computer to the DCHDMI access point using:
    
    - SSID: `Access point SSID`
    
    - Password: `Access point password`

6) When connected to DCHDMI AP, use a browser of your choice and enter `192.168.4.1` in the browsers address bar.

7) The *Web console* should be displayed and setup mode should be started. Just hit `CTRL-D` to exit the setup.

8) Use the `select` command and select the previously downloaded FPGA firmware from the file chooser.

9) `uploadfpga` to upload the file.

10) Use `select` again to select the previously downloaded ESP firmware.

11) `uploadesp` to upload.

12) Use `select` once again to select the Web console file.

13) And upload it using `uploadindex`.

14) Now use `flash` to flash/apply all files to your DCHDMI.

15) `reset` DCHDMI to (re-)start with the new firmware


## Commands

### Basic commands

| Command | Description |
| - | - |
| `config` | Show current DCHDMI configuration data |
| `setup`  | Enter setup mode, to edit configuration |
| `check` | Check for new firmware using ***Firmware Server*** and ***Firmware Version*** configuration data |
| `download` | Download firmware from ***Firmware Server*** to staging area |
| `flash` | Flash firmware from staging area |
| `reset` | Reset DCHDMI. This also resets the Dreamcast |
| `help` | Show available basic commands |
| `helpexpert` | Show available expert commands |
| `clear` | Clear terminal screen |
| `exit` | Exit terminal |


### Expert commands

| Command | Description |
| - | - |
| `check[type]`*(1)* | Check for new *type* firmware using ***Firmware Server*** and ***Firmware Version*** configuration data |
| `select` | Select a file to upload from your computer |
| `upload[type]` | Upload selected file to staging area for *type* |
| `file` | Print information about selected file |
| `download[type]` | Download *type* firmware from ***Firmware Server*** to staging area |
| `flash[type]` | Flash *type* from staging area |
| `reset[type]`*(2)* | Reset *type* |
| `cleanup` | Remove all staged firmware files and forget about previously flashed versions |
| `ls` | List files in filesystem |

*(1)* `[type]`: `fpga`, `esp` or `index`<br>
*(2)* There is no `resetindex` command


### Special commands

| Command | Description |
| - | - |
| `flashfpgasecure` | Flash FPGA firmware while disabling the FPGA |
| `banner` | Print DCHDMI banner |
| `get_mac` | Print MAC address of DCHDMI |
| `flash_chip_size` | Print ESP flash chip size |
| `res_[resolution]`*(1)* | Switch to *resolution* |
| `deinterlace_[deint]`*(2)* | Use deinterlacer *deint* (only works in 15kHz mode) |
| `generate_on` | Generate test video image |
| `generate_off` | Disable test video image |
| `testdata` | Shows test screen |
| `resetpll` | Reset PLL |
| `osd (on,off)` | Show/hide OSD |
| `hq2x (on,off)` | Activate/deactivate HQ2X filter (only in 960p/1080p mode with *Relaxed* firmware active) |
| `240p_offset ([0-9]+)` | Set offset for 240p mode (pixel x2) |
| `osd (on,off)` | Show/hide OSD |
| `spi_flash_erase` | Erase FPGA configuration memory |
| `osdwrite`\ <br>&nbsp;&nbsp;`[column]`\ <br>&nbsp;&nbsp;`[row]`\ <br>&nbsp;&nbsp;`"[text]"` | Write `[text]` to OSD starting at `[column]`/`[row]` |

*(1)* `[resolution]`: `vga`, `480p`, `960p` or `1080p`
<br>*(2)* `[deint]`: `bob` or `passthru`

---
<!--
    /upload/fpga", HTTP_POST
    /upload/esp", HTTP_POST
    /upload/index", HTTP_POST
    /debug", HTTP_GET
    /list-files", HTTP_GET
    /download/fpga", HTTP_GET
    /download/esp", HTTP_GET
    /download/index", HTTP_GET
    /flash/fpga", HTTP_GET
    /flash/esp", HTTP_GET
    /flash/index", HTTP_GET
    /cleanupindex", HTTP_GET
    /cleanup", HTTP_GET
    /resetconfig", HTTP_GET
    /factoryresetall", HTTP_GET
    /flash/secure/fpga", HTTP_GET
    /progress", HTTP_GET
    /flash_size", HTTP_GET
    /reset/fpga", HTTP_GET
    /reset/dc", HTTP_GET
    /reset/all", HTTP_GET
    /reset/pll", HTTP_GET
    /issetupmode", HTTP_GET
    /ping", HTTP_GET
    /setup", HTTP_POST
    /config", HTTP_GET
    /reset/esp", HTTP_ANY
    /test", HTTP_GET
    /osdwrite", HTTP_POST
    /240p_offset", HTTP_POST
    /scanlines", HTTP_POST
    /osd/on", HTTP_GET
    /osd/off", HTTP_GET
    /hq2x/on", HTTP_GET
    /hq2x/off", HTTP_GET
    /res/VGA", HTTP_GET
    /res/480p", HTTP_GET
    /res/960p", HTTP_GET
    /res/1080p", HTTP_GET
    /deinterlace/bob", HTTP_GET
    /deinterlace/passthru", HTTP_GET
    /generate/on", HTTP_GET
    /generate/off", HTTP_GET
    /spi/flash/erase", HTTP_GET
    /clock/config/get", HTTP_GET
    /clock/config/set", HTTP_POST
    /mac/get", HTTP_GET
    /nbp/reset", HTTP_GET
    /testdata2", HTTP_GET
    /testdata", HTTP_GET
-->
<!--
DCHDMI firmware consist of 3 firmware parts:

- FPGA firmware 
    - [`DCxPlus-v2.dc`](https://dc.i74.de/fw/master/DCxPlus-v2.dc)

- ESP firmware
    - [`4MB-firmware.bin`](https://dc.i74.de/esp/master/4MB-firmware.bin)

- index.html (*Web console interface*)
    - [`esp.index.html.gz`](https://dc.i74.de/esp/master/esp.index.html.gz)

-->

