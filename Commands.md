# Web console commands

These are the basic web console commands:

### Basic commands

| Commands | Description |
+----------+-------------+
| `config` | Show current DCHDMI configuration data. |

#### `setup`

Enters setup mode, where you can edit the configuration.

#### `check`

Checks for new firmware using ***Firmware Server*** and ***Firmware Version*** configuration data.

#### `download`

Downloads (new) firmware from ***Firmware Server***.

#### `flash`

Flashes new firmware.

#### `reset`

Resets DCHDMI. This also resets the Dreamcast.

#### help



#### helpexpert


clear                                  
exit


check[type]                            
select                                 
upload[type]                           
    fpga
    esp
    index
file                                   
download[type]                         
flash[type]
    fpga
    esp
    index
"flash",
    "flashfpga",
    "flashfpgasecure",
    "flashesp",
    "flashindex",

reset[type]                            
    fpga
    esp
                                       
cleanup                                
ls                                     
                                       
exit                                   
"details",
    "detailsfpga",
    "detailsesp",
    "detailsindex",
"banner",

==================================
                                       
get_mac
factoryreset
cleanup
resetconfig
flash_chip_size
res_vga
res_480p
res_960p
res_1080p
deinterlace_bob
deinterlace_passthru
generate_on
generate_off
spi_flash_erase
resetpll
testdata
osd (on|off)
hq2x (on|off)
osdwrite ([0-9]+) ([0-9]+) "([^"]*)"
240p_offset ([0-9]+)
debug



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

<!--
DCHDMI firmware consist of 3 firmware parts:

- FPGA firmware 
    - [`DCxPlus-v2.dc`](https://dc.i74.de/fw/master/DCxPlus-v2.dc)

- ESP firmware
    - [`4MB-firmware.bin`](https://dc.i74.de/esp/master/4MB-firmware.bin)

- index.html (*Web console interface*)
    - [`esp.index.html.gz`](https://dc.i74.de/esp/master/esp.index.html.gz)

-->