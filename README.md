# DreamcastHDMI

0. Preface
    - Thanks to: 
        1. [Holguer A Becerra (spanish)](https://sites.google.com/site/ece31289upb/practicas-de-clase/practica-4-sincronizadores/hdmi_de0-nano), [google translated version](https://translate.google.com/translate?sl=es&tl=en&js=y&prev=_t&hl=de&ie=UTF-8&u=https%3A%2F%2Fsites.google.com%2Fsite%2Fece31289upb%2Fpracticas-de-clase%2Fpractica-4-sincronizadores%2Fhdmi_de0-nano&edit-text=)
        2. [FPGA4FUN](http://fpga4fun.com/HDMI.html)
        3. [chipos81/charcole](https://github.com/charcole/NeoGeoHDMI)
        4. [Public.Resource.Org](https://law.resource.org/pub/12tables.html)
    
    I started this project to get familiar with FPGAs and HDMI protocol.

1. Hardware Modifications
    - Video:
        
        Get HSync,VSync, VideoClk and D0-D11 from IC401.

        It's quite tricky to solder kynar wire directly to the video DAC, but with a relatively steady hand it should be ok.
        
        ![Schematic][IC401schematic]
        
        ![Photo][IC401photo]

    - Audio: 


2. Video
    - Video Clock
    - Hsync/Vsync/Enabling 480p
    - Dreamcast 480p (not VGA)
    - 12 bit Encoding

3. Audio
    - Encoding
    - Shielding issues
    
4. HDMI






----------
 
[Technical details](https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/index.html)

[IC401schematic]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/VideoDAConSchematic.png
[IC401photo]: https://media.githubusercontent.com/media/chriz2600/DreamcastHDMI/master/assets/VideoDAC3.JPG