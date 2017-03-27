# DreamcastHDMI

### [Main documentation][maindoc]

---

### News

#### 2017-03-27

##### Cheaper FPGA

Instead of a [DE0 Nano SOC][de0nanosoc] you can also use a [Waveshare CoreEP4CE6 Development Board][CoreEP4CE6]. You can get it for 20-25$ via [Aliexpress][AliCoreEP4CE6]. The project folder for this is [FPGA-CycloneIV]. Because the board does not expose any clock pins, you have to desolder the oscillator and attach the dreamcast clock directly to the board.

![Oscillator][CoreEP4CE6-Oscillator]

##### ADV7513

I also created a board for experimenting with the [Analog Devices ADV7513 HDMI Transmitter][ADV7513] which can be found [here][ADV7513p]. The verilog code for this can be found [here][FPGA-CycloneIV-ADV7513].

##### Direct HDMI output from FPGA

If you want to go the "cheap" DIY route, i've made a PCB for the [LVDS2TMDS][LVDS2TMDSboard] part.

##### Dreamcast video output

Some details about the Dreamcast scaling issue on modern HDTVs: [Video details link]

##### Roadmap

0. ~~Create cheaper solution based on simple FPGA development board.~~
0. Use FPGA to enable 480p mode. Currently I have to plug in a VGA cable ;)
0. Design FPGA board with Cyclone IV FPGA and ADV7513 transmitter. I'm planning to include some RAM to be able to implement 480i as well as basic upscaling later.
0. Design flat flex circuit to connect Dreamcast video DAC and audio DAC to FPGA board.
0. Detailed HOWTOs.

---

[Quartus]: https://www.altera.com/products/design-software/fpga-design/quartus-prime/overview.html
[de0nanosoc]: http://www.terasic.com.tw/cgi-bin/page/archive.pl?Language=English&No=941
[UltraHDMI]: http://ultrahdmi.retroactive.be/
[UltraHDMI Flatflex]: http://cdn3.bigcommerce.com/s-c7bpm05/product_images/theme_images/ultrahdmi_carousel_2.png?t=1478293813
[Holguer A Becerra (spanish)]: https://sites.google.com/site/ece31289upb/practicas-de-clase/practica-4-sincronizadores/hdmi_de0-nano
[google translated version]: https://translate.google.com/translate?sl=es&tl=en&js=y&prev=_t&hl=de&ie=UTF-8&u=https%3A%2F%2Fsites.google.com%2Fsite%2Fece31289upb%2Fpracticas-de-clase%2Fpractica-4-sincronizadores%2Fhdmi_de0-nano&edit-text=
[FPGA4FUN]: http://fpga4fun.com/HDMI.html
[chipos81/charcole]: https://github.com/charcole/NeoGeoHDMI
[Public.Resource.Org]: https://law.resource.org/pub/12tables.html
[Technical Details]: https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/index.html
[Video Details Link]: https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/video.html
[IC401schematic]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/VideoDAConSchematic.png
[IC401photo]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/VideoDAC3.JPG
[IC401solderPoints]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/VideoDACSolderingPoints.png
[DCvideo]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/dc-video.png
[dc-hso]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Dreamcast_Hardware_Specification_Outline.pdf
[ADV7513]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Datasheets/ADV7513.pdf
[PCM1725]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Datasheets/pcm1725.pdf
[LVDS2TMDS]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/LVDS2TMDS.png
[LVDS2TMDS-breadboard]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/LVDS2TMDS2.JPG
[IC303]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/IC303.png
[Overview]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/Overview.JPG
[CoreEP4CE6-Oscillator]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/Waveshare-CoreEP4CE6.png

[HDMI1.3a]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Specs/HDMISpecification13a.pdf
[HDMI1.4]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Specs/HDMI-Specification-1.4.pdf
[EIA-CEA-861-D]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Specs/EIA-CEA-861-D.pdf
[IEC-60958-3]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Specs/is.iec.60958.3.2003.pdf

[CoreEP4CE6]: http://www.waveshare.com/wiki/CoreEP4CE6
[AliCoreEP4CE6]: https://www.aliexpress.com/item/Waveshare-Altera-Cyclone-Board-CoreEP4CE6-EP4CE6E22C8N-EP4CE6-ALTERA-Cyclone-IV-CPLD-FPGA-Development-Core-Board-Full/32643916772.html
[FPGA-CycloneIV]: https://github.com/chriz2600/DreamcastHDMI/tree/master/FPGA-CycloneIV
[FPGA-CycloneIV-ADV7513]: https://github.com/chriz2600/DreamcastHDMI/tree/master/FPGA-CycloneIV-ADV7513

[ADV7513]: http://www.analog.com/en/products/audio-video/analoghdmidvi-interfaces/analog-hdmidvi-display-interfaces/adv7513.html
[ADV7513p]: https://github.com/chriz2600/ADV7513

[LVDS2TMDSboard]: https://github.com/chriz2600/LVDS2TMDS
[maindoc]: https://github.com/chriz2600/DreamcastHDMI/blob/master/Documentation.md
