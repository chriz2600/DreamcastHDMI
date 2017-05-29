# Dreamcast Docker Build/Programming Envirnonment

Docker must be installed: [Docker Homepage][docker]

### Building DreamcastHDMI Firmware

Building dreamcast-hdmi firmware has been tested with windows/mac os x and linux.
On Linux/Mac OS X just run `docker/build`.

```
Usage: ./docker/build <path_to_save_output_files> [<path_to_local_source>]

    path_to_save_output_files:  path to where to save the firmware files
                                (the directory must be accessable by docker)

    path_to_local_source:       optional, if you have local modifications, 
                                this points to your local repository.
                                (the directory must be accessable by docker)
```

### Programming DreamcastHDMI Firmware

Programming dreamcast-hdmi firmware does only work with qemu (kvm) on linux, due to the lack of hyperkit (Mac OS X) and Hyper-V (Windows) allowing USB-Device access. It may also work with docker-machine (which utilizes virtualbox), but I've not tested this. It also runs on Mac OS X and Windows using VMWare and virtualized Linux (so docker is running inside the virtualized Linux and access to USB-Blaster is given to VMWare Linux).

```
Usage: ./docker/program <path_to_file_to_program>

    path_to_file_to_program:    path to programming file (.sof) or URL 
                                pointing to programming file.
                                (the path must be accessable by docker)
                                autobuilds are available on dc.i74.de
```

Link: [dc.i74.de][dc.i74.de]

---

[dc.i74.de]: http://dc.i74.de/
[docker]: https://www.docker.com/