## Firmware versions

All versions are available on the [firmware site][firmware].

The firmware files come in two variants:
- `.jic` Active serial configuration file for use with programmer (e.g. USBBlaster).
- `.rbf` Raw binary configuration file for use with firmware manager.

### master [:link:](https://dc.i74.de/fw/master)

This is the current development branch with support for DCHDMI 1.2


### v1.1.x [:link:](https://dc.i74.de/fw/v1.1.x)

This branch is intended for citrus3000psi **DCHDMI 1.1** ([Mainboard][citrus3000psi-oshpark-mainboard] / [QSB][citrus3000psi-oshpark-qsb]). 
<br>
There will also be a flexible flat cable QSB version available designed by citrus3000psi, which will also work with DCHDMI 1.2

Firmware comes for 3 different FPGAs:

- `DCxPlus-10CL025` _Cyclone 10 LP 10CL025, supports 960p and 1080p_

- `DCxPlus-10CL016` _Cyclone 10 LP 10CL016, supports 960p_

- `DCxPlus-EP4CE6` _Cyclone IV EP4CE6, VGA only_

Releases in this branch:

- [v1.1.0-alpha](https://dc.i74.de/fw/v1.1.0-alpha)


### v0.1 [:link:](https://dc.i74.de/fw/v0.1/)

- `FPGA-CycloneIV-ADV7513-Enhanced_EPCS16` _Waveshare CycloneIV and ADV7513 dev board and ICS664-03 [link][builddoc]._

- `FPGA-CycloneIV-ADV7513_EPCS16` _Waveshare CycloneIV and ADV7513 dev board (without ICS664-03). This version does not recode 480p to VGA._

- `FPGA-CycloneIV_EPCS16` _Waveshare CycloneIV without ADV7513 dev board nor ICS664-03. HDMI output is done via basic LVDS to TMDS [converter][docs]._

---

[master-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/master/download?job=firmware
[v0_2-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/v0.2/download?job=firmware
[firmware]: https://dc.i74.de/
[builddoc]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Build.md
[docs]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Documentation.md
[citrus3000psi-oshpark-mainboard]: https://oshpark.com/shared_projects/N92txcNt
[citrus3000psi-oshpark-qsb]: https://oshpark.com/shared_projects/N0YmRkIu
