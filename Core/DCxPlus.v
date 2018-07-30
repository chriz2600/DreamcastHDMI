
`include "config.inc"

module DCxPlus(
    input wire clock54,
    input wire clock74_175824,
    input wire _hsync,
    input wire _vsync,
    input wire [11:0] data,
    input wire HDMI_INT_N,
    input wire video_mode_480p_n,

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

    output wire status_led
);

wire clock54_net;
wire pll54_locked;
wire hdmi_clock;
wire pll74_locked;

wire [11:0] data_in_counter_x;
wire [11:0] data_in_counter_y;
wire [7:0] dc_blue;
wire [7:0] dc_green;
wire [7:0] dc_red;

wire ram_wren;
wire ram_wrclock;
wire [`RAM_ADDRESS_BITS-1:0] ram_wraddress;
wire [23:0] ram_wrdata;
wire [`RAM_ADDRESS_BITS-1:0] ram_rdaddress;
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

assign clock54_out = clock54_net;
assign status_led = ~adv7513_ready;

// DC config in, ics config out
configuration configurator(
    ._480p_active_n(video_mode_480p_n),
    .line_doubler(_240p_480i_mode),
    .clock_config_S(S)
);

/////////////////////////////////
// PLLs
pll54 pll54(
    .inclk0(clock54),
    .areset(1'b0),
    .c0(clock54_net),
    .locked(pll54_locked)
);

pll74 pll74(
    .inclk0(clock74_175824),
    .areset(1'b0),
    .c0(hdmi_clock),
    .locked(pll74_locked)
);

/////////////////////////////////
// 54/27 MHz area
data video_input(
    .clock(clock54_net),
    .reset(pll54_locked),
    ._hsync(_hsync),
    ._vsync(_vsync),
    .line_doubler(_240p_480i_mode),
    .indata(data),
    .add_line(add_line_mode),
    .blue(dc_blue),
    .counterX(data_in_counter_x),
    .counterY(data_in_counter_y),
    .green(dc_green),
    .red(dc_red)
);

video2ram video2ram(
    .clock(clock54_net),
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
    .wrdata(ram_wrdata)
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
    .FlagOut_clkB(output_trigger));

/////////////////////////////////
// HDMI clock area
ram2video ram2video(
    .starttrigger(output_trigger),
    .clock(hdmi_clock),
    .reset(ram2video_ready),
    .line_doubler(_240p_480i_mode),
    .add_line(add_line_mode),
    .rddata(ram_rddata),
    .hsync(HSYNC),
    .vsync(VSYNC),
    .DrawArea(DE),
    .videoClock(CLOCK),
    .rdaddr(ram_rdaddress),
    .text_rddata(text_rddata),
    .text_rdaddr(text_rdaddr),
    .restart(restart),
    .video_out(VIDEO),
    .enable_osd(enable_osd),
    .highlight_line(highlight_line)
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
    .ready(adv7513_ready),
    .debugData_out(debugData)
);

startup adv7513_startup_delay(
    .clock(hdmi_clock),
    .reset(pll74_locked),
    .ready(adv7513_reset)
);

startup ram2video_startup_delay(
    .clock(hdmi_clock),
    .reset(adv7513_ready),
    .ready(ram2video_ready)
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
    .rst(1'b0),
    .sda(ESP_SDA),
    .scl(ESP_SCL),
    .ram_dataIn(text_wrdata),
    .ram_wraddress(text_wraddr),
    .ram_wren(text_wren),
    .enable_osd(enable_osd),
    .debugData(debugData),
    .controller_data(controller_data),
    .highlight_line(highlight_line)
);

maple mapleBus(
    .clk(hdmi_clock),
    .pin1(MAPLE_PIN1),
    .pin5(MAPLE_PIN5),
    .controller_data(controller_data)
);

endmodule
