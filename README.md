# DreamcastHDMI

## 0. Preface

### Thanks 

1. [Holguer A Becerra (spanish)], [google translated version]
2. [FPGA4FUN]
3. [chipos81/charcole]
4. [Public.Resource.Org]

I started this project to get familiar with FPGAs and HDMI protocol. What I have now is working 480p video directly from dreamcasts GPU (Holly) with digital audio via HDMI.

### What's missing?

1. Only 480p(VGA) capable games are working, no 480i support yet
2. EDID support
3. Upscaling
4. OSD

## 1. Hardware Modifications

Luckily, the dreamcast uses an external VideoDAC (IC401), so we can tap into the signals here:

*VideoDAC on Schematic:*

![Schematic][IC401schematic]

- **VSync** (VSYNC) (pin 52)
- **HSync** (HSYNC) (pin 53)
- **VClk** (VCLK) (pin 54) 

    Video clock: double the pixel clock (54Mhz for 480p), because RGB values are transmitted in two clock cycles. (See below)

- **D0-D11** (pin 56, pins 1-11)

    From [Dreamcast Hardware Specification Outline (page 37)][dc-hso]:

    > From HOLLY, the 24bit RGB image information (each RGB 8bit) is sent in units of 12 bit to the Digital-Video-Encoder (video DAC/encoder). The original 24 bit image data is divided into 12bit(RGB [11:0]) of MSB (RGB [11:0] = R [7:0], G[7:4]) and 12 bit of LSB (RGB [11:0] = G [3:0], B [7:0]). Then it is sent to the 54 Hz clock (which is double the 27Hz VGA pixel-clock) in a synchronised manner where it alternates between MSB and LSB. For details refer to a separate specification design document.

![Dreamcast video][DCvideo]

For the audio part we can tap into the AudioDAC ([PCM1725][PCM1725]) (IC303). The sampling rate is 44.1kHz.

*AudioDAC on Schematic:*

![Schematic][IC303]

- **LRC** (DLRCK) (pin 1) Audio LR Clock (LVTTL)
- **AData** (DSD) (pin 2) Audio Data (LVTTL)
- **BCK** (DBCK) (pin 3) Audio Clock (LVTTL)
- **AClk** (DSCK) (pin 14) 256Fs audio clock signal (LVTTL)

*Soldering points on Dreamcast mainboard:*

![Photo][IC401solderPoints]

*Kynar wire soldered to VideoDAC:*

![Photo][IC401photo]

It's quite tricky to solder kynar wire directly to the video DAC, because the round wire tends to slip between the legs of the chip, but with a relatively steady hand - mine is not :) - it should be manageable. Lots of flux is the key. It should be possible to design a flatflex cable which is soldered directly to the VideoDAC (like the [UltraHDMI] is doing [UltraHDMI Flatflex])

## 2. Video 
    
The dreamcast is generating 480p (not VGA)
- Dreamcast 480p (not VGA)

## 3. Audio

- Encoding
- Shielding issues
    
## 4. HDMI

Currently I am creating the HDMI output directly via the FPGA's LVDS outputs and a simple LVDS to TMDS converter and a breadboard HDMI plug, which is far from optimal, but working ok for 480p. 

According to [Holguer A Becerra][Holguer A Becerra (spanish)] (top of site) it should even work without any level conversion "since most CML (similar to TMDS) receivers already have AC / DC coupling", but without I didn't had any luck on my AV receiver.

*LVDS to TMDS Converter:*

![LVDS to TMDS Converter][LVDS2TMDS] 

*LVDS to TMDS Converter on breadboard:*

![LVDS to TMDS Converter on breadboard][LVDS2TMDS-breadboard]

For a real product a real HDMI transmitter should be used (e.g. [ADV7513][ADV7513]).





---

[UltraHDMI]: http://ultrahdmi.retroactive.be/
[UltraHDMI Flatflex]: http://cdn3.bigcommerce.com/s-c7bpm05/product_images/theme_images/ultrahdmi_carousel_2.png?t=1478293813
[Holguer A Becerra (spanish)]: https://sites.google.com/site/ece31289upb/practicas-de-clase/practica-4-sincronizadores/hdmi_de0-nano
[google translated version]: https://translate.google.com/translate?sl=es&tl=en&js=y&prev=_t&hl=de&ie=UTF-8&u=https%3A%2F%2Fsites.google.com%2Fsite%2Fece31289upb%2Fpracticas-de-clase%2Fpractica-4-sincronizadores%2Fhdmi_de0-nano&edit-text=
[FPGA4FUN]: http://fpga4fun.com/HDMI.html
[chipos81/charcole]: https://github.com/charcole/NeoGeoHDMI
[Public.Resource.Org]: https://law.resource.org/pub/12tables.html

[Technical Details]: https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/index.html
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