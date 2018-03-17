## Firmware versions

All versions are available on the [firmware site][firmware].

The firmware files come in two variants:
- `.jic` Active serial configuration file for use with programmer (e.g. USBBlaster).
- `.rbf` Raw binary configuration file for use with firmware manager.

### v0.1 [:link:](http://dc.i74.de/fw/v0.1/)

- `FPGA-CycloneIV-ADV7513-Enhanced_EPCS16`

  *optional firmware manager support*

  Version for Waveshare CycloneIV board with ADV7513 dev board and ICS664-03 as described [here][builddoc].

- `FPGA-CycloneIV-ADV7513_EPCS16`

  *jic only version, no firmware manager support*

  Version for Waveshare CycloneIV board with ADV7513 dev board (without ICS664-03).
  This version does not recode 480p to VGA.

- `FPGA-CycloneIV_EPCS16`

  *jic only version, no firmware manager support*

  Version for Waveshare CycloneIV board without ADV7513 dev board nor ICS664-03.
  HDMI output is done via basic LVDS to TMDS converter, as described [here][docs].

### master [:link:](http://dc.i74.de/fw/master/) / v0.2 [:link:](http://dc.i74.de/fw/v0.2/)

This version is intended for citrus3000psi DCHDMI 1.1 ([Mainboard][citrus3000psi-oshpark-mainboard] / [QSB][citrus3000psi-oshpark-qsb]). There is also a firmware for DCHDMI 1.0, mainly because I have a working 1.0 board at the moment :)

[master firmware files][master-artifact]<br>
[v0.2 firmware files][v0_2-artifact]

- `DCxPlus-10CL025`

  Version for DCHDMI 1.1 with Cyclone 10 LP 10CL025. This version is supposed to support 960p and 1080p in a later revision.

- `DCxPlus-10CL016`

  Version for DCHDMI 1.1 with Cyclone 10 LP 10CL016.

- `DCxPlus-EP4CE6`

  Version for DCHDMI 1.1 with Cyclone IV EP4CE6.

- `DCxPlus-EP4CE6-old_pinout`

  Version for DCHDMI 1.0 with Cyclone IV EP4CE6.

---

[master-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/master/download?job=firmware
[v0_2-artifact]: https://gitlab.com/chriz2600/DreamcastHDMI/-/jobs/artifacts/v0.2/download?job=firmware
[firmware]: http://dc.i74.de/
[builddoc]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Build.md
[docs]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Documentation.md
[citrus3000psi-oshpark-mainboard]: https://oshpark.com/shared_projects/N92txcNt
[citrus3000psi-oshpark-qsb]: https://oshpark.com/shared_projects/N0YmRkIu
