## DCHDMI LED status patterns

*LED status is only available, when `Opt. reset mode` in Web console `setup` is set to `led` (which is the default)*

#### **LED off**

No power to DCHDMI or FPGA configuration is corrupt.

*If the FPGA configuration has been corrupted, just wait for 2 minutes, as it will be reflashed by the ESP.*

#### **LED dim on**

No clock from Dreamcast. 

*Check connection to `VCLK`*

#### **LED rapid blinking**

No proper sync signal is detected (It's normal to see this at startup/reset)

*Check connections to `VSYNC` and `HSYNC`*

#### **LED slow glow**

Dreamcast clock and sync signals are fine, no cable plugged into HDMI port.

*Plug in HDMI cable or replace with another cable*

#### **LED fully on**

Normal operation.

#### **LED rapid intermittent blinking**

Indicates video test pattern generation. Only used in test rig or using the `generate_on` console command.

---

![vdac](https://github.com/chriz2600/DreamcastHDMI/raw/experimental/assets/vdac.png)