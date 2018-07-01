# Dreamcast Docker Build/Programming Envirnonment

Docker must be installed: [Docker Homepage][docker]

### Building DreamcastHDMI Firmware

#### Linux/Mac OSX

Simply use `./docker/build`:
```
Usage: ./docker/build <path_to_save_output_files> [<path_to_local_source>]

    path_to_save_output_files:  path to where to save the firmware files
                                (the directory must be accessable by docker)

    path_to_local_source:       optional, if you have local modifications, 
                                this points to your local repository.
                                (the directory must be accessable by docker)
```

#### Windows Power Shell

1) Build from github master branch, no source checkout needed:
    ```
    $PathToSaveGeneratedFirmware = Convert-Path C:\DCHDMI-Firmware
    docker run --rm -it `
        -v ${PathToSaveGeneratedFirmware}:/srv/ `
        chriz2600/dreamcast-hdmi:latest /root/build
    ```

2) Build from local checkout:
    ```
    $PathToSaveGeneratedFirmware = Convert-Path C:\DCHDMI-Firmware
    $PathToLocalSource = Convert-Path C:\DreamcastHDMI
    docker run --rm -it `
        -e "SKIP_PULL=true" `
        -v ${PathToSaveGeneratedFirmware}:/srv/ `
        -v ${PathToLocalSource}:/root/DreamcastHDMI `
        chriz2600/dreamcast-hdmi:latest /root/build
    ```

### Programming DreamcastHDMI Firmware via DCHDMI

DCHDMI allows flashing of the firmware over it's web interface. If you have set up WiFi already, just click [here](http://dc-firmware-manager.local).

#### Complete firmware upgrade from official firmware server

This performs a full system upgrade, including FPGA configuration data, ESP firmware and web interface.

```
download
flashfpga
reset
restart
```

#### Upgrade from locally build firmware

This performs an FPGA configuration data upgrade.

```
select
uploadfpga
flashfpga
reset
```

### Programming DreamcastHDMI Firmware via JTAG

Programming dreamcast-hdmi firmware does only work on linux, due to the lack of hyperkit (Mac OS X) and Hyper-V (Windows) allowing USB-Device access. It may also work with docker-machine (which utilizes virtualbox), but I've not tested this. It also runs on Mac OS X and Windows using VMWare and virtualized Linux (so docker is running inside the virtualized Linux and access to USB-Blaster is given to VMWare Linux).

#### Linux

```
Usage: ./docker/program <path_to_file_to_program>

    path_to_file_to_program:    path to programming file (.sof) or URL 
                                pointing to programming file.
                                (the path must be accessable by docker)
                                autobuilds are available on dc.i74.de
```

#### Windows

If 

```
```

Link: [dc.i74.de][dc.i74.de]

---

[dc.i74.de]: http://dc.i74.de/
[docker]: https://www.docker.com/