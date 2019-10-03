# Serial Port

DCHDMI outputs a lot if information over the serial console. This can be really helpful, e.g. if you do not get any video output, but want to connect to DCHDMI's access point.

#### What do I need to connect to the DCHDMI serial port?

A 3.3V serial port adapter.

- [SparkFun Beefy 3 - FTDI Basic Breakout](https://www.sparkfun.com/products/13746) will fit directly to the DCHDMI serial port using a 2.54mm header and is able to supply 3.3V to DCHDMI.

    You don't have to solder the 2.54mm header to the board, just plug it into the serial port adapter and hold it at an angle applying light pressure, so it makes good contact to the holes on the DCHDMI board.

    Be sure the markings on the adpater line up with the markings on the DCHDMI board. (the adapter side with the USB port is facing away from the DCHDMI board)

    *No external power supply is needed, when using this serial port adapter.*

- [FT232RL 3.3V 5.5V FTDI USB to TTL Serial Adapter Module](https://www.amazon.com/XCSOURCE-FT232RL-Adapter-Arduino-TE203/dp/B00HSX3CXE/)

    If you are using this adapter make sure the jumper is set to **3.3V** and the adapter side with the USB port is facing towards the DCHDMI board.

    *This might also work without using an external power supply, but it will exceed the 50mA current the FT232 can provide. The FT232 IC will get hot, when you're powering DCHDMI this way, and the serial port adapter might get damaged.*

- If using your own serial port adapter make sure it's working at 3.3V! Otherwise DCHDMI might get damaged. (It's not 5V tolerant!)

    Also make sure, that your adapter is capable of supplying 200mA@3.3V or you have to use an external power supply.

**If powering DCHDMI from a serial port adapter `3.3V`, `GND`, `RX` and `TX` have to be connected.**

**If connecting to DCHDMI, when mounted inside the console and connected to the flat flex, only `GND`, `RX` and `TX` have to be connected.**

**Keep in mind that the serial port adapter `RX` has to be connected to the `TX` on DCHDMI and vice versa (It's a serial port :)).**

#### Example output on first start

```
>> ESP starting... v4.2
>> SDK:2.2.1(cfd48f3)/Core:2.5.2=20502000/lwIP:STABLE-2_1_2_RELEASE/glue:1.1-7-g82abda3/BearSSL:a143020
>> Setting up SPIFFS...
>> _readFile: /etc/reset/mode:[led] (default)
>> Setting up reset mode: 0
--> ERROR_END_I2C_TRANSACTION
<-- FINISHED_I2C_TRANSACTION
   success 1st command: f2 00 (21)
   success 2nd command: ff 00 (21)
   retry loops needed: 21
>> _readFile: /etc/video/mode:[CableDetect] (default)
>> _readFile: /etc/video/resolution:[VGA] (default)
>> _readFile: /etc/deinterlace/mode/480i:[bob] (default)
>> _readFile: /etc/deinterlace/mode/576i:[bob] (default)
>> Setting up output resolution: 3
   mapResolution: resd: 03 tres: 03 crd: 00 cdm: 00|00
   success 1st command: 83 03 (1)
   success 2nd command: f0 00 (1)
   retry loops needed: 1
>> _readFile: /etc/scanlines/active:[off] (default)
>> _readFile: /etc/scanlines/intensity:[175] (default)
>> _readFile: /etc/scanlines/oddeven:[even] (default)
>> _readFile: /etc/scanlines/thickness:[thin] (default)
>> Setting up scanlines:
   success 1st command: 88 57 (1)
   success 2nd command: 89 80 (1)
   retry loops needed: 1
>> _readFile: /etc/240p/offset:[0] (default)
>> _readFile: /etc/vga/offset:[1] (default)
   success 1st command: 90 00 (1)
   success 2nd command: 90 00 (1)
   retry loops needed: 1
   success 1st command: 94 00 (1)
   success 2nd command: 94 00 (1)
   retry loops needed: 1
>> _readFile: /etc/upscaling/mode:[0] (default)
   success 1st command: 92 00 (1)
   success 2nd command: 92 00 (1)
   retry loops needed: 1
>> _readFile: /etc/color/space:[0] (default)
   success 1st command: 93 00 (1)
   success 2nd command: 93 00 (1)
   retry loops needed: 1
>> Setting up task manager...
>> Reading stored values...
>> _readFile: /etc/ssid:[] (default)
>> _readFile: /etc/password:[] (default)
>> _readFile: /etc/ota_pass:[] (default)
>> _readFile: /etc/firmware_server:[dc.i74.de] (default)
>> _readFile: /etc/firmware_variant:[std] (default)
>> _readFile: /etc/firmware_version:[master] (default)
>> _readFile: /etc/http_auth_user:[dchdmi] (default)
>> _readFile: /etc/http_auth_pass:[] (default)
>> _readFile: /etc/conf_ip_addr:[] (default)
>> _readFile: /etc/conf_ip_gateway:[] (default)
>> _readFile: /etc/conf_ip_mask:[] (default)
>> _readFile: /etc/conf_ip_dns:[] (default)
>> _readFile: /etc/hostname:[dc-firmware-manager] (default)
>> _readFile: /etc/keyblayout:[us] (default)
>> _readFile: /etc/protected/mode:[off] (default)
>> Checking video mode controller override...
no valid controller packet found within timeout
>> No ssid, starting AP mode...
AP_NameChar: DCHDMI-1450
AP password: TJDN94ELr0
>> SSID:   DCHDMI-1450
>> AP-PSK: TJDN94ELr0
>> Setting up HTTP server...
>> httpAuthUser: dchdmi
>> httpAuthPass: Xx7u3G4hMB
>> Ready.
```

**Here you find the generated access point credentials `SSID`/`AP-PSK` as well as the web console credentials `httpAuthUser`/`httpAuthPass`.**