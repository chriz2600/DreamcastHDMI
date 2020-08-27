
`include "config.inc"

module DCxPlus(
    input wire clock54,
    input wire clock74_175824,
    input wire _hsync,
    input wire _vsync,
    input wire [11:0] data,
    input wire HDMI_INT_N,
    inout wire video_mode_480p_n,

    output wire clock54_out,

    inout wire SDAT,
    inout wire SCLK,

    output wire HSYNC,
    output wire VSYNC,
    output wire DE,
    output wire CLOCK,
    output wire [23:0] VIDEO,
    output wire [3:0] S,

    inout ESP_SDA,
    input ESP_SCL,

    input MAPLE_PIN1,
    input MAPLE_PIN5,

    inout wire status_led_nreset,
    inout wire DC_NRESET
);

    wire clock54_net;
    wire pll54_locked;

    // hdmi pll
    wire hdmi_clock;
    wire pll_hdmi_areset;
    wire pll_hdmi_scanclk;
    wire pll_hdmi_scandata;
    wire pll_hdmi_scanclkena;
    wire pll_hdmi_configupdate;
    wire pll_hdmi_locked;
    wire pll_hdmi_scandataout;
    wire pll_hdmi_scandone;

    // hdmi pll reconfig
    wire pll_hdmi_reconfig;
    wire pll_hdmi_write_from_rom;
    wire pll_hdmi_rom_data_in;
    wire [7:0] pll_hdmi_rom_address_out;
    wire pll_hdmi_write_rom_ena;

    // hdmi pll_reconf rom
    wire reconf_fifo_rdempty;
    wire [7:0] reconf_fifo_q;
    wire reconf_fifo_rdreq;
    wire reconf_fifo_wrreq;
    wire reconf_fifo_wrfull;

    wire [7:0] reconf_data;

    // ---------------------------
    wire [11:0] data_in_counter_x;
    wire [11:0] data_in_counter_y;
    wire [7:0] dc_blue;
    wire [7:0] dc_green;
    wire [7:0] dc_red;

    wire ram_wren;
    wire [`RAM_WIDTH-1:0] ram_wraddress;
    wire [23:0] ram_wrdata;
    wire [`RAM_WIDTH-1:0] ram_rdaddress;
    wire [23:0] ram_rddata;

    wire buffer_ready_trigger;
    wire output_trigger;

    wire _240p_480i_mode;
    wire add_line_mode;
    wire is_pal_mode;

    //wire ram2video_ready;

    wire [9:0] text_rdaddr;
    wire [7:0] text_rddata;
    wire [9:0] text_wraddr;
    wire [7:0] text_wrdata;
    wire text_wren;
    wire enable_osd;
    wire enable_osd_out;
    wire [7:0] highlight_line;
    //DebugData debugData;
    ControllerData controller_data;
    KeyboardData keyboard_data;
    HDMIVideoConfig hdmiVideoConfig;
    DCVideoConfig _dcVideoConfig;
    DCVideoConfig dcVideoConfig;
    Scanline scanline;
    wire forceVGAMode;
    wire resync;
    wire [23:0] pinok;
    wire [23:0] pinok_out;
    wire [23:0] timingInfo;
    wire [23:0] timingInfo_out;
    wire [23:0] rgbData;
    wire [23:0] rgbData_out;
    wire [23:0] conf240p;
    wire [23:0] conf240p_out;
    wire force_generate;

    wire generate_video;
    wire generate_timing;
    wire [7:0] video_gen_data;
    wire activateHDMIoutput;
    wire hq2x;
    wire [1:0] colorspace;
    wire [1:0] colorspace_data;
    wire hq2x_out;
    wire r2v_f;
    wire fullcycle;
    wire reset_dc;
    wire reset_opt;
    wire [7:0] reset_conf;

    wire control_clock;
    //wire control_clock_2;
    wire hdmi_int_reg;
    wire hpd_detected;
    wire _config_changed;
    wire config_changed;
    wire _nonBlackPixelReset;
    wire nonBlackPixelReset;

    wire [11:0] nonBlackPos1;
    wire [11:0] nonBlackPos2;
    wire [11:0] nonBlackPos1_out;
    wire [11:0] nonBlackPos2_out;

    wire [23:0] color_space_explorer;
    wire [23:0] color_space_explorer_out;

    wire [7:0] color_config_data;
    wire [7:0] color_config_data_out;
    wire resetpll;

    assign clock54_out = clock54_net;

    // DC config in, ics config out
    configuration configurator(
        .clock(control_clock),
        // inputs
        .dcVideoConfig(_dcVideoConfig),
        .forceVGAMode(forceVGAMode),
        .force_generate(control_force_generate_out /*force_generate*/),
        // output
        .config_changed(_config_changed),
        .line_doubler(line_doubler_sync2 /*_240p_480i_mode*/),
        // output pins
        .clock_config_S(S),
        ._480p_active_n(video_mode_480p_n)
    );

    /////////////////////////////////
    // PLLs
    pll54 pll54(
        .inclk0(clock54),
        .areset(resetpll),
        .c0(clock54_net),
        .locked(pll54_locked)
    );

    pll_hdmi pll_hdmi(
        .inclk0(clock74_175824),
        .c0(hdmi_clock),
        .c1(CLOCK),
        .locked(pll_hdmi_locked),

        .areset(pll_hdmi_areset),
        .scanclk(pll_hdmi_scanclk),
        .scandata(pll_hdmi_scandata),
        .scanclkena(pll_hdmi_scanclkena),
        .configupdate(pll_hdmi_configupdate),

        .scandataout(pll_hdmi_scandataout),
        .scandone(pll_hdmi_scandone)
    );

    wire pll_reconf_busy;
    pll_hdmi_reconf	pll_hdmi_reconf(
        .clock(control_clock),
        .reconfig(pll_hdmi_reconfig),
        .busy(pll_reconf_busy),
        .data_in(9'b0),
        .counter_type(4'b0),
        .counter_param(3'b0),

        .pll_areset_in(pll54_lockloss /*|| pll_hdmi_lockloss*/),

        .pll_scandataout(pll_hdmi_scandataout),
        .pll_scandone(pll_hdmi_scandone),
        .pll_areset(pll_hdmi_areset),
        .pll_configupdate(pll_hdmi_configupdate),
        .pll_scanclk(pll_hdmi_scanclk),
        .pll_scanclkena(pll_hdmi_scanclkena),
        .pll_scandata(pll_hdmi_scandata),

        .write_from_rom(pll_hdmi_write_from_rom),
        .rom_data_in(pll_hdmi_rom_data_in),
        .rom_address_out(pll_hdmi_rom_address_out),
        .write_rom_ena(pll_hdmi_write_rom_ena)
    );

    wire is_pll54_locked;
    wire is_pll_hdmi_locked;
    Signal_CrossDomain isPll54Locked(.SignalIn_clkA(pll54_locked), .clkB(control_clock), .SignalOut_clkB(is_pll54_locked));
    Signal_CrossDomain isPllHdmiLocked(.SignalIn_clkA(pll_hdmi_locked), .clkB(control_clock), .SignalOut_clkB(is_pll_hdmi_locked));

    pll_hdmi_reconfig reconf_rom(
        .clock(control_clock),
        .address(pll_hdmi_rom_address_out),
        .read_ena(pll_hdmi_write_rom_ena),
        .pll_reconf_busy(pll_reconf_busy || ~is_pll54_locked || ~is_pll_hdmi_locked),
        .data(reconf_data/*reconf_data_clock54*/),

        .q(pll_hdmi_rom_data_in),
        .reconfig(pll_hdmi_reconfig),
        .trigger_read(pll_hdmi_write_from_rom)
    );

    /////////////////////////////////
    // 54/27 MHz area

    wire [7:0] reconf_data_clock54;

    data_cross #(
        .WIDTH($bits(DCVideoConfig) + 3 + $bits(conf240p) + $bits(video_gen_data) + $bits(color_config_data))
    ) dcVideoConfig_cross(
        .clkIn(control_clock),
        .clkOut(clock54_net),
        .dataIn({ _dcVideoConfig, _config_changed, line_doubler_sync2, _nonBlackPixelReset, conf240p, video_gen_data, color_config_data }),
        .dataOut({ dcVideoConfig, config_changed, _240p_480i_mode, nonBlackPixelReset, conf240p_out, { 6'bzzzzzz, generate_video, generate_timing }, color_config_data_out })
    );

    dc_video_reconfig dc_video_configurator(
        .clock(control_clock),
        .data_in(reconf_data),
        .dcVideoConfig(_dcVideoConfig),
        .forceVGAMode(forceVGAMode)
    );

    data video_input(
        .clock(clock54_net),
        .reset(~pll54_locked || config_changed),
        ._hsync(_hsync),
        ._vsync(_vsync),
        .line_doubler(_240p_480i_mode),
        .generate_video(generate_video),
        .generate_timing(generate_timing),
        .indata(data),
        .add_line(add_line_mode),
        .is_pal(is_pal_mode),
        .resync(resync),
        .force_generate(force_generate),
        .blue(dc_blue),
        .counterX(data_in_counter_x),
        .counterY(data_in_counter_y),
        .green(dc_green),
        .red(dc_red),
        .pinok(pinok),
        .timingInfo(timingInfo),
        .rgbData(rgbData),
        .conf240p(conf240p_out),
        .nonBlackPos1(nonBlackPos1),
        .nonBlackPos2(nonBlackPos2),
        .nonBlackPixelReset(nonBlackPixelReset),
        .color_space_explorer(color_space_explorer)
    );

    video2ram video2ram(
        .clock(clock54_net),
        .nreset(~resync),
        .line_doubler(_240p_480i_mode),
        .is_pal(is_pal_mode),
        .B(dc_blue),
        .counterX(data_in_counter_x),
        .counterY(data_in_counter_y),
        .G(dc_green),
        .R(dc_red),
        .wren(ram_wren),
        .starttrigger(buffer_ready_trigger),
        .wraddr(ram_wraddress),
        .wrdata(ram_wrdata),
        .dcVideoConfig(dcVideoConfig),
        .color_config_data(color_config_data_out)
    );

    /////////////////////////////////
    // clock domain crossing
    ram video_buffer(
        .wrclock(clock54_net),
        .wren(ram_wren),
        .wraddress(ram_wraddress),
        .data(ram_wrdata),

        .rdclock(hdmi_clock),
        .rdaddress(ram_rdaddress),
        .q(ram_rddata)
    );

    Flag_CrossDomain trigger(
        .clkA(clock54_net),
        .FlagIn_clkA(buffer_ready_trigger),
        .clkB(hdmi_clock),
        .FlagOut_clkB(output_trigger)
    );

    /////////////////////////////////
    // HDMI clock area
    reg prev_resync_out = 0;
    reg resync_out = 0;
    reg resync_signal = 1;
    wire line_doubler_sync;
    wire line_doubler_sync2;
    wire add_line_sync;
    wire is_pal_sync;
    wire [7:0] reconf_data_hdmi;
    wire is_interlaced;

    Flag_CrossDomain #(
        .INIT_STATE(1'b1)
    ) rsync_trigger (
        .clkA(clock54_net),
        .FlagIn_clkA(resync),
        .clkB(hdmi_clock),
        .FlagOut_clkB(resync_out)
    );

    always @(posedge hdmi_clock) begin
        if (~prev_resync_out && resync_out) begin
            resync_signal <= 1'b1;
        end else if (prev_resync_out && ~resync_out) begin
            resync_signal <= 1'b0;
        end
        prev_resync_out <= resync_out;
    end

    Signal_CrossDomain lineDoubler(
        .SignalIn_clkA(_240p_480i_mode),
        .clkB(hdmi_clock),
        .SignalOut_clkB(line_doubler_sync)
    );

    data_cross reconfDataHdmi(
        .clkIn(control_clock),
        .clkOut(hdmi_clock),
        .dataIn(reconf_data),
        .dataOut(reconf_data_hdmi)
    );

    hdmi_video_reconfig hdmi_video_configurator(
        .clock(hdmi_clock),
        .data_in(reconf_data_hdmi),
        .r2v_f(r2v_f),
        .hdmiVideoConfig(hdmiVideoConfig)
    );

    Flag_CrossDomain enable_osd_cross(
        .clkA(control_clock),
        .FlagIn_clkA(enable_osd),
        .clkB(hdmi_clock),
        .FlagOut_clkB(enable_osd_out)
    );

    Flag_CrossDomain enable_hq2x_cross(
        .clkA(control_clock),
        .FlagIn_clkA(hq2x),
        .clkB(hdmi_clock),
        .FlagOut_clkB(hq2x_out)
    );

    Signal_CrossDomain isInterlacedSig(
        .SignalIn_clkA(~add_line_mode && _240p_480i_mode),
        .clkB(hdmi_clock),
        .SignalOut_clkB(is_interlaced)
    );

`ifdef std
    `include "DCxPlus-std.v"
`elsif hq2x
    `include "DCxPlus-hq2x.v"
`endif

    text_ram text_ram_inst(
        .rdclock(hdmi_clock),
        .rdaddress(text_rdaddr),
        .q(text_rddata),

        .wrclock(control_clock),
        .wraddress(text_wraddr),
        .data(text_wrdata),
        .wren(text_wren)
    );

    ////////////////////////////////////////////////////////////////////////
    // dreamcast reset and control, also ADV7513 I2C control
    ////////////////////////////////////////////////////////////////////////
    // reset clock circuit
    osc control_clock_gen(
        .oscena(1'b1),
        .clkout(control_clock)
    );
    ////////////////////////////////////////////////////////

    reg pll54_lockloss;
    reg pll_hdmi_lockloss;

    edge_detect pll54_lockloss_check(
        .async_sig(~pll54_locked),
        .clk(control_clock),
        .rise(pll54_lockloss)
    );

    edge_detect pll_hdmi_lockloss_check(
        .async_sig(~pll_hdmi_locked),
        .clk(control_clock),
        .rise(pll_hdmi_lockloss)
    );

    data_cross #(
        .WIDTH(24 + 24 + 24 + 12 + 12 + 24)
    ) pinok_cross(
        .clkIn(clock54_net),
        .clkOut(control_clock),
        .dataIn({ pinok, timingInfo, rgbData, nonBlackPos1, nonBlackPos2, color_space_explorer }),
        .dataOut({ pinok_out, timingInfo_out, rgbData_out, nonBlackPos1_out, nonBlackPos2_out, color_space_explorer_out })
    );

    Signal_CrossDomain addLine(
        .SignalIn_clkA(add_line_mode),
        .clkB(control_clock),
        .SignalOut_clkB(add_line_sync)
    );

    Signal_CrossDomain isPAL(
        .SignalIn_clkA(is_pal_mode),
        .clkB(control_clock),
        .SignalOut_clkB(is_pal_sync)
    );

    reg[31:0] counter = 0;
    reg[31:0] counter2 = 0;
    reg dc_nreset_reg = 1'b1;
    reg opt_nreset_reg = 1'b1;
    reg status_led_nreset_reg = 1'bz;
    reg control_resync_out;
    reg control_force_generate_out;

    assign DC_NRESET = (dc_nreset_reg ? 1'bz : 1'b0);
    assign status_led_nreset = status_led_nreset_reg;

    wire slowGlow_out;
    wire fastGlow_out;
    wire dim_out;

    LEDglow #(
        .BITPOS(26)
    ) slowGlow (
        .clk(control_clock),
        .LED(slowGlow_out)
    );

    LEDglow #(
        .BITPOS(22)
    ) fastGlow (
        .clk(control_clock),
        .LED(fastGlow_out)
    );

    LEDdim dimled (
        .clk(control_clock),
        .LED(dim_out)
    );

    Flag_CrossDomain #(
        .INIT_STATE(1'b1)
    ) control_rsync (
        .clkA(clock54_net),
        .FlagIn_clkA(resync),
        .clkB(control_clock),
        .FlagOut_clkB(control_resync_out)
    );

    Flag_CrossDomain control_force_generate(
        .clkA(clock54_net),
        .FlagIn_clkA(force_generate),
        .clkB(control_clock),
        .FlagOut_clkB(control_force_generate_out)
    );

    reg [31:0] led_counter = 0;

`ifdef DIAG_MODE
    always @(posedge control_clock) begin
        if (adv_i2c_working) begin
            status_led_nreset_reg <= ~slowGlow_out;
        end else if (~pll_ready) begin
            status_led_nreset_reg <= ~dim_out;
        end else begin
            status_led_nreset_reg <= ~fastGlow_out;
        end
    end
`else
    always @(posedge control_clock) begin
        if (reset_conf == 2'd0) begin // LED
            if (~pll_ready) begin
                status_led_nreset_reg <= ~dim_out;
            end else if (control_resync_out) begin
                status_led_nreset_reg <= ~fastGlow_out;
            end else if (control_force_generate_out) begin
                if (led_counter[24]) begin
                    status_led_nreset_reg <= ~fastGlow_out;
                end else begin
                    status_led_nreset_reg <= 1'b1;
                end
            end else begin
                if (adv7513_ready) begin
                    status_led_nreset_reg <= 1'b0;
                end else begin
                    status_led_nreset_reg <= ~slowGlow_out;
                end
            end
        end else if (reset_conf == 2'd1 || reset_conf == 2'd3) begin // GDEMU || MODE
            status_led_nreset_reg <= (opt_nreset_reg ? 1'bz : 1'b0);
        end else if (reset_conf == 2'd2) begin // USBGDROM
            status_led_nreset_reg <= (dc_nreset_reg ? 1'bz : 1'b0);
        end else begin
            status_led_nreset_reg <= 1'bz;
        end
        led_counter <= led_counter + 1'b1;
    end
`endif

    localparam RESET_HOLD_TIME = 32'd32_000_000;

    always @(posedge control_clock) begin
        if (reset_dc) begin
            counter <= 0;
            dc_nreset_reg <= 1'b0;
        end else begin
            counter <= counter + 1;
            if (counter == RESET_HOLD_TIME) begin
                dc_nreset_reg <= 1'b1;
            end
        end
    end

    always @(posedge control_clock) begin
        if (reset_opt) begin
            counter2 <= 0;
            opt_nreset_reg <= 1'b0;
        end else begin
            counter2 <= counter2 + 1;
            if (counter2 == RESET_HOLD_TIME) begin
                opt_nreset_reg <= 1'b1;
            end
        end
    end

    wire [7:0] highlight_line_in;
    Scanline scanline_in;

    data_cross highlight_line_cross(
        .clkIn(control_clock),
        .clkOut(hdmi_clock),
        .dataIn(highlight_line_in),
        .dataOut(highlight_line)
    );

    data_cross #(
        .WIDTH($bits(Scanline))
    ) scanline_cross(
        .clkIn(control_clock),
        .clkOut(hdmi_clock),
        .dataIn(scanline_in),
        .dataOut(scanline)
    );

    i2cSlave i2cSlave(
        .clk(control_clock),
        .rst(1'b0),
        .sda(ESP_SDA),
        .scl(ESP_SCL),

        .ram_dataIn(text_wrdata),
        .ram_wraddress(text_wraddr),
        .ram_wren(text_wren),
        .enable_osd(enable_osd),
        //.debugData(debugData),
        .highlight_line(highlight_line_in),
        .reconf_data(reconf_data),
        .scanline(scanline_in),

        .controller_data(controller_data),
        .keyboard_data(keyboard_data),
        .reset_dc(reset_dc),
        .reset_opt(reset_opt),
        .reset_conf(reset_conf),
        .pinok(pinok_out),
        .timingInfo(timingInfo_out),
        .rgbData(rgbData_out),
        .conf240p(conf240p),
        .add_line(add_line_sync),
        .line_doubler(line_doubler_sync2),
        .is_pal(is_pal_sync),
        .video_gen_data(video_gen_data),
        .activateHDMIoutput(activateHDMIoutput),
        .hq2x(hq2x),
        .colorspace(colorspace_data),
        .force_generate(control_force_generate_out),

        .pll_adv_lockloss_count(pll_adv_lockloss_count),
        .hpd_low_count(hpd_low_count),
        .pll54_lockloss_count(pll54_lockloss_count),
        .pll_hdmi_lockloss_count(pll_hdmi_lockloss_count),
        .control_resync_out_count(control_resync_out_count),
        .monitor_sense_low_count(monitor_sense_low_count),
        .testdata({ 
            HDMI_INT_N, 
            adv7513_reconf, 
            startup_ready, 
            pll_ready, 
            ram2video_fullcycle, 
            status_led_nreset, 
            is_pll54_locked, 
            is_pll_hdmi_locked, 
            _config_changed, 
            line_doubler_sync2,
            control_resync_out,
            forceVGAMode,
            video_mode_480p_n,
            3'b0 
        }),
        .clock_config_data(clock_config_data),
        .color_config_data(color_config_data),
        .nonBlackPos1(nonBlackPos1_out),
        .nonBlackPos2(nonBlackPos2_out),
        .nonBlackPixelReset(_nonBlackPixelReset),
        .color_space_explorer(color_space_explorer_out),
        .resetpll(resetpll)
    );

    maple mapleBus(
        .clk(control_clock),
        .reset(1'b0),
        .pin1(MAPLE_PIN1),
        .pin5(MAPLE_PIN5),
        .controller_data(controller_data),
        .keyboard_data(keyboard_data)
    );

    ////////////////////////////////////////////////////////////////////////

    wire [31:0] pll_adv_lockloss_count;
    wire [31:0] hpd_low_count;
    wire [31:0] monitor_sense_low_count;
    reg [31:0] pll54_lockloss_count = 0;
    reg [31:0] pll_hdmi_lockloss_count = 0;
    reg [31:0] control_resync_out_count = 0;
    reg prev_control_resync_out = 0;

    always @(posedge control_clock) begin
        if (pll54_lockloss) begin
            pll54_lockloss_count <= pll54_lockloss_count + 1'b1;
        end
        if (pll_hdmi_lockloss) begin
            pll_hdmi_lockloss_count <= pll_hdmi_lockloss_count + 1'b1;
        end
        prev_control_resync_out <= control_resync_out;
        if (~prev_control_resync_out && control_resync_out) begin
            control_resync_out_count <= control_resync_out_count + 1'b1;
        end
    end

    ////////////////////////////////////////////////////////////////////////
        // // I2C master clock divisions
        // // divider = (pixel_clock / bus_clk) / 4; bus_clk = 400_000
        // // divider[31:24] = div4
        // // divider[23:16] = div3
        // // divider[15:8]  = div2
        // // divider[7:0]   = div1
        // reg [31:0] divider;

    wire startup_ready;
    wire adv7513_ready;
    ADV7513Config adv7513Config;
    wire reconf_fifo2_rdempty;
    wire [7:0] reconf_fifo2_q;
    wire reconf_fifo2_rdreq;
    wire pll_ready;
    wire ram2video_fullcycle;

    Signal_CrossDomain pll_hdmi_locked_check_adv(
        .SignalIn_clkA(pll54_locked),
        .clkB(control_clock),
        .SignalOut_clkB(pll_ready)
    );

    Signal_CrossDomain ram2video_fullcycle_check_adv(
        .SignalIn_clkA(fullcycle),
        .clkB(control_clock),
        .SignalOut_clkB(ram2video_fullcycle)
    );

    startup adv7513_startup_delay(
        .clock(control_clock),
        .nreset(1'b1),
        .ready(startup_ready),
        .startup_delay(32'd_64_000_000)
    );

    wire adv_i2c_working;
    wire adv7513_reconf;
    wire [7:0] clock_config_data;
    wire [7:0] clock_data;
    adv7513_reconfig reconf_adv(
        .clock(control_clock),
        .data_in(reconf_data),
        .clock_config_data(clock_config_data),
        .colorspace_in(colorspace_data),
        .adv7513Config(adv7513Config),
        .clock_data_out(clock_data),
        .colorspace_out(colorspace),
        .adv7513_reconf(adv7513_reconf)
    );

    ADV7513 adv7513(
        .clk(control_clock),
        .reset(startup_ready),
        .hdmi_int(HDMI_INT_N & ~adv7513_reconf),
        .output_ready(startup_ready & pll_ready & ram2video_fullcycle & ~adv7513_reconf),
        .sda(SDAT),
        .scl(SCLK),
        .ready(adv7513_ready),
        .adv7513Config(adv7513Config),
        .clock_data(clock_data),
        .colorspace(colorspace),
        .hdmi_int_reg(hdmi_int_reg),
        .hpd_detected(hpd_detected),
        .pll_adv_lockloss_count(pll_adv_lockloss_count),
        .hpd_low_count(hpd_low_count),
        .monitor_sense_low_count(monitor_sense_low_count),
        .i2c_working(adv_i2c_working)
    );

endmodule
