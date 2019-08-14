wire hqx_fullcycle;
wire [9:0] hqx_text_rdaddr;
wire [`RAM_WIDTH-1:0] hqx_ram_rdaddress;
wire hqx_hsync;
wire hqx_vsync;
wire hqx_de;
wire [23:0] hqx_video;

ram2video_f ram2video_f(
    .clock(hdmi_clock),
    .reset(~pll_hdmi_locked || resync_signal),
    .starttrigger(output_trigger),
    .fullcycle(hqx_fullcycle),
    .hq2x(hq2x_out),
    .scanline(scanline),

    .rdaddr(hqx_ram_rdaddress),
    .rddata(ram_rddata),

    .text_rdaddr(hqx_text_rdaddr),
    .text_rddata(text_rddata),
    .enable_osd(enable_osd_out),
    .highlight_line(highlight_line),

    //input line_doubler,
    .hdmiVideoConfig(hdmiVideoConfig),

    // video output
    .video_out(hqx_video),
    .hsync(hqx_hsync),
    .vsync(hqx_vsync),
    .DrawArea(hqx_de)
);

wire std_fullcycle;
wire [9:0] std_text_rdaddr;
wire [`RAM_WIDTH-1:0] std_ram_rdaddress;
wire std_hsync;
wire std_vsync;
wire std_de;
wire [23:0] std_video;

ram2video ram2video(
    .clock(hdmi_clock),
    .reset(~pll_hdmi_locked || resync_signal),
    .starttrigger(output_trigger),
    .fullcycle(std_fullcycle),
    .is_interlaced(is_interlaced),

    .rdaddr(std_ram_rdaddress),
    .rddata(ram_rddata),

    .text_rdaddr(std_text_rdaddr),
    .text_rddata(text_rddata),
    .enable_osd(enable_osd_out),
    .highlight_line(highlight_line),

    .hdmiVideoConfig(hdmiVideoConfig),
    .line_doubler(line_doubler_sync),
    .scanline(scanline),

    .video_out(std_video),
    .hsync(std_hsync),
    .vsync(std_vsync),
    .DrawArea(std_de)
);

busmux #(.WIDTH(1 + 10 /* text ram */ + `RAM_WIDTH + 1 + 1 + 1 + 24 /* video */)) r2v_mux(
    .dataa({ std_fullcycle, std_text_rdaddr, std_ram_rdaddress, std_hsync, std_vsync, std_de, std_video }),
    .datab({ hqx_fullcycle, hqx_text_rdaddr, hqx_ram_rdaddress, hqx_hsync, hqx_vsync, hqx_de, hqx_video }),
    .result({ fullcycle, text_rdaddr, ram_rdaddress, HSYNC, VSYNC, DE, VIDEO }),
    .sel(r2v_f && ~line_doubler_sync)
);
