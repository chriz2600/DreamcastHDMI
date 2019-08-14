
ram2video ram2video(
    .clock(hdmi_clock),
    .reset(~pll_hdmi_locked || resync_signal),
    .starttrigger(output_trigger),
    .fullcycle(fullcycle),
    .is_interlaced(is_interlaced),

    .rdaddr(ram_rdaddress),
    .rddata(ram_rddata),

    .text_rdaddr(text_rdaddr),
    .text_rddata(text_rddata),
    .enable_osd(enable_osd_out),
    .highlight_line(highlight_line),

    .hdmiVideoConfig(hdmiVideoConfig),
    .line_doubler(line_doubler_sync),
    .scanline(scanline),

    .video_out(VIDEO),
    .hsync(HSYNC),
    .vsync(VSYNC),
    .DrawArea(DE)
);
