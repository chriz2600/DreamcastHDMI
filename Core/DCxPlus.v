
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
wire [7:0] reconf_fifo_data;
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
wire ram_wrclock;
wire [14:0] ram_wraddress;
wire [23:0] ram_wrdata;
wire [14:0] ram_rdaddress;
wire [23:0] ram_rddata;

wire buffer_ready_trigger;
wire output_trigger;

wire _240p_480i_mode;
wire add_line_mode;

wire adv7513_reset;
wire adv7513_ready;
wire ram2video_ready;

wire [9:0] text_rdaddr;
wire [7:0] text_rddata;
wire [9:0] text_wraddr;
wire [7:0] text_wrdata;
wire text_wren;
wire restart;
wire enable_osd;
wire [7:0] highlight_line;
DebugData debugData;
ControllerData controller_data;
HDMIVideoConfig hdmiVideoConfig;
DCVideoConfig dcVideoConfig;
Scanline scanline;
wire forceVGAMode;
wire pll54_lockloss;
wire pll_hdmi_lockloss;
wire resetPLL;
wire resync;
wire resync_rise;

wire generate_video;
wire generate_timing;
wire fullcycle;
wire reset_dc;
wire reset_clock;

assign clock54_out = clock54_net;
assign status_led_nreset = ~fullcycle;

// DC config in, ics config out
configuration configurator(
    .clock(clock54_net),
    .dcVideoConfig(dcVideoConfig),
    ._480p_active_n(video_mode_480p_n),
    .forceVGAMode(forceVGAMode),
    .line_doubler(_240p_480i_mode),
    .clock_config_S(S)
);

/////////////////////////////////
// PLLs
pll54 pll54(
    .inclk0(clock54),
    .areset(1'b0),
    //.areset(pll54_lockloss),
    .c0(clock54_net),
    .locked(pll54_locked)
);

edge_detect pll54_lockloss_check(
    .async_sig(pll54_locked),
    .clk(clock54),
    .fall(pll54_lockloss)
);

edge_detect pll_hdmi_lockloss_check(
    .async_sig(pll_hdmi_locked),
    .clk(clock54),
    .fall(pll_hdmi_lockloss)
);

pll_hdmi pll_hdmi(
    .inclk0(clock74_175824),
    .c0(hdmi_clock),
    .c1(CLOCK),
    .locked (pll_hdmi_locked),

    .areset(pll_hdmi_areset),
    .scanclk(pll_hdmi_scanclk),
    .scandata(pll_hdmi_scandata),
    .scanclkena(pll_hdmi_scanclkena),
    .configupdate(pll_hdmi_configupdate),

    .scandataout(pll_hdmi_scandataout),
    .scandone(pll_hdmi_scandone)
);

pll_hdmi_reconf	pll_hdmi_reconf(
    .clock(clock54_net),

    .reconfig(pll_hdmi_reconfig),

    // not connected
    //.read_param(read_param_sig),
    //.write_param(write_param_sig),
    //.reset(reset_sig),
    //.reset_rom_address(reset_rom_address_sig),
    //.busy(busy_sig),
    //.data_out(data_out_sig),

    .data_in(9'b0),
    .counter_type(4'b0),
    .counter_param(3'b0),

    //.pll_areset_in(1'b0),
    .pll_areset_in(resetPLL || ~pll54_locked),

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

reconf_rom reconf_rom(
    .clock(clock54_net),
    .address(pll_hdmi_rom_address_out),
    .read_ena(pll_hdmi_write_rom_ena),
    .q(pll_hdmi_rom_data_in),
    .reconfig(pll_hdmi_reconfig),
    .rdempty(reconf_fifo_rdempty),
    .fdata(reconf_fifo_q),
    .rdreq(reconf_fifo_rdreq),
    .trigger_read(pll_hdmi_write_from_rom),
    .forceVGAMode(forceVGAMode),
    .resetPLL(resetPLL),
    .dcVideoConfig(dcVideoConfig),
    .generate_video(generate_video),
    .generate_timing(generate_timing)
);

reconf_fifo	reconf_fifo(
    .rdclk(clock54_net),
    .rdreq(reconf_fifo_rdreq),
    .rdempty(reconf_fifo_rdempty),
    .q(reconf_fifo_q),

    .wrclk(hdmi_clock),
    .data(reconf_fifo_data),
    .wrreq(reconf_fifo_wrreq),
    .wrfull(reconf_fifo_wrfull)
);

trigger_reconf trigger_reconf(
    .clock(hdmi_clock),
    .wrfull(reconf_fifo_wrfull),
    .data_in(reconf_data),
    .data(reconf_fifo_data),
    .wrreq(reconf_fifo_wrreq)
);

/////////////////////////////////
// 54/27 MHz area
data video_input(
    .clock(clock54_net),
    .reset(pll54_locked),
    ._hsync(_hsync),
    ._vsync(_vsync),
    .line_doubler(_240p_480i_mode),
    .generate_video(generate_video),
    .generate_timing(generate_timing),
    .indata(data),
    .add_line(add_line_mode),
    .resync(resync),
    .blue(dc_blue),
    .counterX(data_in_counter_x),
    .counterY(data_in_counter_y),
    .green(dc_green),
    .red(dc_red)
);

video2ram video2ram(
    .clock(clock54_net),
    .nreset(~resync),
    .line_doubler(_240p_480i_mode),
    .B(dc_blue),
    .counterX(data_in_counter_x),
    .counterY(data_in_counter_y),
    .G(dc_green),
    .R(dc_red),
    .wren(ram_wren),
    .wrclock(ram_wrclock),
    .starttrigger(buffer_ready_trigger),
    .wraddr(ram_wraddress),
    .wrdata(ram_wrdata),
    .dcVideoConfig(dcVideoConfig)
);

/////////////////////////////////
// clock domain crossing
ram video_buffer(
    .wren(ram_wren),
    .wrclock(ram_wrclock),
    .rdclock(hdmi_clock),
    .data(ram_wrdata),
    .rdaddress(ram_rdaddress),
    .wraddress(ram_wraddress),
    .q(ram_rddata)
);

Flag_CrossDomain trigger(
    .clkA(ram_wrclock),
    .FlagIn_clkA(buffer_ready_trigger),
    .clkB(hdmi_clock),
    .FlagOut_clkB(output_trigger)
);

/////////////////////////////////
// HDMI clock area
reg prev_resync_out = 0;
reg resync_out = 0;

Flag_CrossDomain rsync_trigger(
    .clkA(clock54_net),
    .FlagIn_clkA(resync),
    .clkB(hdmi_clock),
    .FlagOut_clkB(resync_out)
);

always @(posedge hdmi_clock) begin
    if (~prev_resync_out && resync_out) begin
        resync_rise <= 1'b1;
    end else begin
        resync_rise <= 1'b0;
    end
    prev_resync_out <= resync_out;
end

ram2video ram2video(
    .starttrigger(output_trigger),
    .clock(hdmi_clock),
    .reset(ram2video_ready && ~resync_rise),
    .line_doubler(_240p_480i_mode),
    .add_line(add_line_mode),
    .rddata(ram_rddata),
    .hsync(HSYNC),
    .vsync(VSYNC),
    .DrawArea(DE),
    .rdaddr(ram_rdaddress),
    .text_rddata(text_rddata),
    .text_rdaddr(text_rdaddr),
    .restart(restart),
    .video_out(VIDEO),
    .enable_osd(enable_osd),
    .highlight_line(highlight_line),
    .hdmiVideoConfig(hdmiVideoConfig),
    .scanline(scanline),
    .fullcycle(fullcycle)
);

ADV7513 adv7513(
    .clk(hdmi_clock),
    .reset(adv7513_reset),
    .hdmi_int(HDMI_INT_N),
    .VSYNC(VSYNC),
    .DE(DE),
    .sda(SDAT),
    .scl(SCLK),
    .restart(restart),
    .resync_rise(resync_rise),
    .ready(adv7513_ready),
    .debugData_out(debugData),
    .hdmiVideoConfig(hdmiVideoConfig)
);

startup adv7513_startup_delay(
    .clock(hdmi_clock),
    .nreset(pll_hdmi_locked),
    .ready(adv7513_reset),
    .startup_delay(hdmiVideoConfig.startup_delay)
);

startup ram2video_startup_delay(
    .clock(hdmi_clock),
    .nreset(pll_hdmi_locked),
    .ready(ram2video_ready),
    .startup_delay(hdmiVideoConfig.startup_delay)
);

text_ram text_ram_inst(
    .clock(hdmi_clock),
    .data(text_wrdata),
    .rdaddress(text_rdaddr),
    .wraddress(text_wraddr),
    .wren(text_wren),
    .q(text_rddata)
);

i2cSlave i2cSlave(
    .clk(hdmi_clock),
    .rst(~ram2video_ready),
    .sda(ESP_SDA),
    .scl(ESP_SCL),
    .ram_dataIn(text_wrdata),
    .ram_wraddress(text_wraddr),
    .ram_wren(text_wren),
    .enable_osd(enable_osd),
    .debugData(debugData),
    .controller_data(controller_data),
    .highlight_line(highlight_line),
    .reconf_data(reconf_data),
    .hdmiVideoConfig(hdmiVideoConfig),
    .scanline(scanline),
    .reset_dc(reset_dc)
);

maple mapleBus(
    .clk(hdmi_clock),
    .pin1(MAPLE_PIN1),
    .pin5(MAPLE_PIN5),
    .controller_data(controller_data)
);

// reset clock circuit
osc reset_clock_gen(
    .oscena(1'b1),
    .clkout(reset_clock)
);

////////////////////////////////////////////////////////////////////////
// dreamcast reset
////////////////////////////////////////////////////////////////////////
Flag_CrossDomain reset_trigger(
    .clkA(hdmi_clock),
    .FlagIn_clkA(reset_dc),
    .clkB(reset_clock),
    .FlagOut_clkB(reset_dc_out)
);

reg[31:0] counter = 0;
reg dc_nreset_reg = 1'b1;
wire reset_dc_out;

assign DC_NRESET = dc_nreset_reg ? 1'bz : 1'b0;

always @(posedge reset_clock) begin
    if (reset_dc_out) begin
        counter <= 0;
        dc_nreset_reg <= 1'b0;
    end else begin
        counter <= counter + 1;
        if (counter == 8_000_000) begin /* 100ms@80MHz, 133ms@60MHz, ... */
            dc_nreset_reg <= 1'b1;
        end
    end
end
////////////////////////////////////////////////////////////////////////

endmodule
