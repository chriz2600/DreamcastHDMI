# DreamcastHDMI

## 0. Preface

### Thanks 

1. [Holguer A Becerra (spanish)](https://sites.google.com/site/ece31289upb/practicas-de-clase/practica-4-sincronizadores/hdmi_de0-nano), [google translated version](https://translate.google.com/translate?sl=es&tl=en&js=y&prev=_t&hl=de&ie=UTF-8&u=https%3A%2F%2Fsites.google.com%2Fsite%2Fece31289upb%2Fpracticas-de-clase%2Fpractica-4-sincronizadores%2Fhdmi_de0-nano&edit-text=)
2. [FPGA4FUN](http://fpga4fun.com/HDMI.html)
3. [chipos81/charcole](https://github.com/charcole/NeoGeoHDMI)
4. [Public.Resource.Org](https://law.resource.org/pub/12tables.html)

I started this project to get familiar with FPGAs and HDMI protocol. What I have now is working 480p video directly from dreamcasts GPU (Holly) with digital audio via HDMI.

### What's missing

1. Only 480p(VGA) capable games are working, no 480i support yet
2. EDID support
3. Upscaling 

## 1. Hardware Modifications

### Video
    
Luckily, the dreamcast uses an external VideoDAC (IC401), so we can tap into the signals here:

*VideoDAC on Schematic:*

![Schematic][IC401schematic]

- VSync (pin 52)
- HSync (pin 53)
- VideoClk (pin 54) 

    Double the pixel clock (54Mhz for 480p), because RGB values are transmitted in two clock cycles. 
    (See "Dreamcast Video" graphic below)

- D0-D11 (pin 56, pins 1-11)

    From [Dreamcast Hardware Specification Outline (page 37)][dc-hso]:

    > From HOLLY, the 24bit RGB image information (each RGB 8bit) is sent in units of 12 bit to the Digital-Video-Encoder (video DAC/encoder). The original 24 bit image data is divided into 12bit(RGB [11:0]) of MSB (RGB [11:0] = R [7:0], G[7:4]) and 12 bit of LSB (RGB [11:0] = G [3:0], B [7:0]). Then it is sent to the 54 Hz clock (which is double the 27Hz VGA pixel-clock) in a synchronised manner where it alternates between MSB and LSB. For details refer to a separate specification design document.

![Dreamcast video][DCvideo]

It's quite tricky to solder kynar wire directly to the video DAC, because the round wire tends to slip between the legs of the chip, but with a relatively steady hand (mine is not) it should be manageable. Lots of flux is the key.

*Kynar wire soldered to VideoDAC:*

![Photo][IC401photo]

### Audio

- DSCK: 256Fs clock signal to external DAC LVTTL

- DSD: 44.1K Digital Audio Data to external DAC LVTTL

- DBCK: 44.1K Digital Audio Clock to external DAC LVTTL

- DLRCK: 44.1K Digital Audio LR Clock to external DAC LVTTL

## 2. Video 
    
The dreamcast is generating 480p (not VGA)
- Dreamcast 480p (not VGA)

## 3. Audio

- Encoding
- Shielding issues
    
## 4. HDMI






---
 
[Technical details]: (https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/index.html)
[IC401schematic]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/VideoDAConSchematic.png
[IC401photo]: https://media.githubusercontent.com/media/chriz2600/DreamcastHDMI/master/assets/VideoDAC3.JPG
[DCvideo]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/dc-video.png
[dc-hso]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Dreamcast_Hardware_Specification_Outline.pdf