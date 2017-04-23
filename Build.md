# Howto Build Dreamcast HDMI

### Step 0: Parts used

1. Dreamcast
2. [Waveshare CoreEP4CE6 development board][CoreEP4CE6]
3. [ICS664-03] digitial video clock source on SSOP adapter
4. [ADV7513 dev board][ADV7513p]

Schematic/Diagram: 
- [PDF][schematics1]
- [SVG][schematics1s] 

Schematic of the FPGA design:
- [PNG][schematics2]

### Step 1: Dreamcast

You need to solder to several points on the DC motherboard. AClk is currently not used. For the audio connections, I also used nearby ground vias to create twisted pairs to avoid interference with other signals.

![step01]

### Step 2: Waveshare CoreEP4CE6

In order to use the Waveshare Core EP4CE6 FPGA board for Dreamcast HDMI, some changes to the board are necessary, because it does not expose the Cyclone IV's PLL clock pins to it's header connectors.

First, you have to remove the 50 MHz oscillator from the board and solder a wire to pin 23 (**clock54** in schematic) of the FPGA. You can solder either directly to the FPGA or use the pad from the oscillator as shown below.

![step02a]

Then you have to connect another wire to pin 25 (**clock74_175824** in schematic) (pin 24 would also work, but had to be changed in the FPGA project). As pin 25 is not connected to anything on the board, the wire has to soldered directly to the FPGA.

![step02b]

### Step 3: ICS664-03 digitial video clock source

I soldered the ICS664-03 to an SSOP to DIP socket and used it on a breadboard.

![step03] 

### Step 4: 

After all is connected together, you get something like this :)

![step04]


[step01]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/01-VideoDACSolderingPoints.png
[step02a]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/02-DC-clock-in.png
[step02b]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/03-ICS-clock-in.png
[step03]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/04-ICS664-3.png
[step04]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/project-with-ics664.JPG

[schematics1]: https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/05-DreamcastHDMI.pdf
[schematics1s]: https://rawgit.com/chriz2600/DreamcastHDMI/master/assets/05-DreamcastHDMI.svg
[schematics2]: https://github.com/chriz2600/DreamcastHDMI/raw/master/assets/06-DCxPlus-Schematic.png

[CoreEP4CE6]: http://www.waveshare.com/wiki/CoreEP4CE6
[ICS664-03]: https://github.com/chriz2600/DreamcastHDMI/raw/master/Documents/Datasheets/IDT_664-03_DST_20100514.pdf
[ADV7513p]: https://github.com/chriz2600/ADV7513
