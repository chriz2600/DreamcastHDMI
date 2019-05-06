## DCHDMI LED status patterns

*LED status is only available, when `Opt. reset mode` in console setup is set to `led`*

#### **LED off**

No clock from Dreamcast. 

*Check connection to `VCLK`*

#### **LED rapid blinking**

No proper sync signal is detected (It's normal to see this at startup/reset)

*Check connections to `VSYNC` and `HSYNC`*

#### **LED slow glow**

Dreamcast clock and sync signals are fine, no cable plugged into HDMI port.

*Plug in HDMI cable or replace with another cable*

#### **LED on**

Normal operation.

#### **LED rapid intermittent blinking**

Indicates video test pattern generation. Only used in test rig or using the `generate_on` console command.

---

![vdac](https://github.com/chriz2600/DreamcastHDMI/raw/hq2x-experimental/assets/vdac.png)