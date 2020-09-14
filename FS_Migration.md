# Filesystem Migration

`v4.7` supports two filesystems, [SPIFFS](https://github.com/pellepl/spiffs) (no longer maintained and deprecated) and [LittleFS](https://github.com/ARMmbed/littlefs).

If you have updated from an older firmware, your DCDigital will still use **SPIFFS**. But it's possible to migrate from **SPIFFS** to **LittleFS**.

#### Step 1

Upgrade firmware to `v4.7`

#### Step 2

Make sure you have [web console](Web_console.md) access. It's recommended to set a web access password using the `setup` command.
It's also recommended to only perform the filesystem update, if you're able to [recover from a soft bricked system](Recovery.md).

#### Step 3

The `ls` command shows, which filesystem is currently used:

```
DCDigital> ls
---------------------------------
FSImpl: SPIFFS                     <---- HERE!
---------------------------------
Size    Filename
---------------------------------
 690194 /firmware.dc
 521472 /firmware.bin
  83752 /esp.index.html.gz
  83752 /index.html.gz
     64 /etc/firmware_version
     64 /etc/http_auth_pass
     64 /etc/ota_pass
     64 /etc/password
     64 /etc/ssid
     32 /esp.index.html.gz.md5
     32 /etc/last_esp_flash_md5
     32 /etc/last_flash_md5
     32 /etc/last_flash_spi_md5
     32 /firmware.bin.md5
     32 /firmware.dc.md5
     32 /index.html.gz.md5
     16 /etc/video/mode
     16 /etc/video/resolution
      8 /etc/last_flash_spi_pages
---------------------------------
1449984 of 3121152 bytes used
```

#### Step 4

Start the migration process by using the `migratefs` command.

- Configuration is preserved during filesystem migration, but the web console's `index.html` is too big to fit into the memory of the ESP8266 during the transition.

- Therefor you will be send to a download of the compressed `index.html.gz`. Please save the file, as you have to upload it again later.

- Then, you have to acknoledge the download on the web console.

- The filesystem migration will be started.

- After this is done, you find yourself on a page, where you can upload the previously downloaded `index.html.gz`.

- You end up in the web console again. Use the `ls` command to confirm the `FSImpl` row says: `LittleFS`.

[![DCDigital Filesystem Migration](http://dc.i74.de/fs_mig_guide.png)](https://www.youtube.com/watch?v=T64nVHV1DeE "DCDigital Filesystem Migration")
