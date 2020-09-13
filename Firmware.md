# Firmware

All versions are available on the [firmware site][firmware].

The firmware consists of 3 parts:

#### FPGA firmware: https://dc.i74.de/fw/

FPGA firmware comes in 3 different file types:

- `.dc` Binary configuration for WiFi/Web console update.

    - `DCxPlus-v2.dc` 
    
        Newer style firmware package containing both `std` and `hq2x` firmware.
        This is used in firmware > v4.0.

    - `DCxPlus-default.dc`
    
        Legacy firmware package for firmware < v4.0.

- `.jic` Active serial configuration file for use with programmer (e.g. USBBlaster).

    **Obsolete** as the production DCHDMI does not have the JTAG connector on board.

    - `DCxPlus-hq2x.jic` Standard firmware

    - `DCxPlus-std.jic` Relaxed firmware

- `.rbf` Raw binary configuration file.

    - `DCxPlus-hq2x.rbf` Standard firmware

    - `DCxPlus-std.rbf` Relaxed firmware

    These files are used to create the `.dc` file(s)

#### ESP firmware: https://dc.i74.de/esp/

- `4MB-firmware.bin` ESP firmware.

- `4MB-littlefs.bin` ESP Filesystem image. ***Used for initial programming.***

#### ESP web console: https://dc.i74.de/esp/

- `esp.index.html.gz` Web console `index.html`

#### Files used by WiFi update

- `https://dc.i74.de/fw/<branch>/DCxPlus-v2.dc` (> v4.0)

    or `https://dc.i74.de/fw/<branch>/DCxPlus-default.dc` (< v4.0)

#### Important

If you want to manually update your firmware as described [here](Web_console.md#i-dont-want-to-connect-my-dchdmi-to-my-wifi-network-how-do-i-update-manually), make sure alle 3 files are from the same branch.

#### Available firmware versions (branches) (aka `Firmware Version` in Web console setup)

##### master

Stable branch. This is the recommended setting for most users.

- FPGA: https://dc.i74.de/fw/master
- ESP/Web console: https://dc.i74.de/esp/master

##### develop

Development branch. Release candidates are released here.

- FPGA: https://dc.i74.de/fw/develop
- ESP/Web console: https://dc.i74.de/esp/develop

##### experimental

Experimental branch. Alpha/Beta versions are released here. **Use with caution!**

- FPGA: https://dc.i74.de/fw/experimental
- ESP/Web console: https://dc.i74.de/esp/experimental

##### bleeding

Bleeding edge branch. Daily builds and features in development is done here.<br>
**Use with extreme caution, as these may require restoring from a bricked console via serial port!**

- FPGA: https://dc.i74.de/fw/bleeding
- ESP/Web console: https://dc.i74.de/esp/bleeding

##### vX.Y

Previously released firmware versions.

- FPGA: https://dc.i74.de/fw/vX.Y
- ESP/Web console: https://dc.i74.de/esp/vX.Y

For a list of all versions:

- FPGA: https://dc.i74.de/fw/
- ESP/Web console: https://dc.i74.de/esp/

##### hq2x-\<branch> / std-\<branch>

These are legacy branches, if doing a WiFi update from a v2.3.x or v3.0.x firmware.

---

[master-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/master/download?job=firmware
[v0_2-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/v0.2/download?job=firmware
[firmware]: https://dc.i74.de/
[builddoc]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Build.md
[docs]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Documentation.md
[citrus3000psi-oshpark-mainboard]: https://oshpark.com/shared_projects/N92txcNt
[citrus3000psi-oshpark-qsb]: https://oshpark.com/shared_projects/N0YmRkIu
