//////////////////////////////////////////////////////////////////////
////                                                              ////
//// registerInterface.v                                          ////
////                                                              ////
//// This file is part of the i2cSlave opencores effort.
//// <http://www.opencores.org/cores//>                           ////
////                                                              ////
//// Module Description:                                          ////
//// You will need to modify this file to implement your 
//// interface.
//// Add your control and status bytes/bits to module inputs and outputs,
//// and also to the I2C read and write process blocks  
////                                                              ////
//// To Do:                                                       ////
//// 
////                                                              ////
//// Author(s):                                                   ////
//// - Steve Fielding, sfielding@base2designs.com                 ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
////                                                              ////
//// Copyright (C) 2008 Steve Fielding and OPENCORES.ORG          ////
////                                                              ////
//// This source file may be used and distributed without         ////
//// restriction provided that this copyright statement is not    ////
//// removed from the file and that any derivative work contains  ////
//// the original copyright notice and the associated disclaimer. ////
////                                                              ////
//// This source file is free software; you can redistribute it   ////
//// and/or modify it under the terms of the GNU Lesser General   ////
//// Public License as published by the Free Software Foundation; ////
//// either version 2.1 of the License, or (at your option) any   ////
//// later version.                                               ////
////                                                              ////
//// This source is distributed in the hope that it will be       ////
//// useful, but WITHOUT ANY WARRANTY; without even the implied   ////
//// warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR      ////
//// PURPOSE. See the GNU Lesser General Public License for more  ////
//// details.                                                     ////
////                                                              ////
//// You should have received a copy of the GNU Lesser General    ////
//// Public License along with this source; if not, download it   ////
//// from <http://www.opencores.org/lgpl.shtml>                   ////
////                                                              ////
//////////////////////////////////////////////////////////////////////
//
`include "i2cSlave_define.v"
`include "config.inc"

module registerInterface (
    input clk,
    input[7:0] addr,
    input[7:0] dataIn,
    input writeEn,
    output[7:0] dataOut,

    output[7:0] ram_dataIn,
    output[9:0] ram_wraddress,
    output ram_wren,
    output enable_osd,
    output[7:0] highlight_line,
    output[7:0] reconf_data,
    output[7:0] video_gen_data,
    output Scanline scanline,
    output [23:0] conf240p,
    output reset_dc,
    output reset_opt,
    output[7:0] reset_conf,
    output activateHDMIoutput,
    output hq2x,
    output [1:0] colorspace,
    input [23:0] pinok,
    input [23:0] timingInfo,
    input [23:0] rgbData,
    input add_line,
    input line_doubler,
    input is_pal,
    input force_generate,
    input ControllerData controller_data,
    input KeyboardData keyboard_data,
    input [31:0] pll_adv_lockloss_count,
    input [31:0] hpd_low_count,
    input [31:0] pll54_lockloss_count,
    input [31:0] pll_hdmi_lockloss_count,
    input [31:0] control_resync_out_count,
    input [31:0] monitor_sense_low_count,
    input [15:0] testdata,
    output [7:0] clock_config_data,
    output [7:0] color_config_data,
    input [11:0] nonBlackPos1,
    input [11:0] nonBlackPos2,
    output nonBlackPixelReset,
    input [23:0] color_space_explorer,
    output resetpll
);

reg [2:0] addr_offset = 3'b000;
reg [7:0] dataOut_reg;
reg [9:0] wraddress_reg;
reg wren;
reg enable_osd_reg = 1'b0;
reg [7:0] highlight_line_reg = 255;
reg [7:0] reconf_data_reg = 0;
reg [7:0] video_gen_data_reg;
reg reset_dc_reg = 1'b0;
reg reset_opt_reg = 1'b0;
reg nonBlackPixelReset_reg = 1'b0;
reg resetpll_reg = 1'b0;

Scanline scanline_reg = { 9'h100, 1'b0, 1'b0, 1'b0 };
reg [23:0] conf240p_reg = 24'd20;
reg [7:0] reset_conf_reg = 8'hFF; // initialize with 'inactive'
reg activateHDMIoutput_reg = 0;
reg hq2x_reg = 0;
reg [1:0] colorspace_reg;
reg [7:0] clock_config_data_reg = 3;
reg [7:0] color_config_data_reg = `GAMMA_1_0 | `RGB888;

assign dataOut = dataOut_reg;
assign ram_wraddress = wraddress_reg;
assign ram_dataIn = dataIn;
assign ram_wren = wren;
assign enable_osd = enable_osd_reg;
assign highlight_line = highlight_line_reg;
assign scanline = scanline_reg;
assign reconf_data = reconf_data_reg;
assign video_gen_data = video_gen_data_reg;
assign reset_dc = reset_dc_reg;
assign reset_opt = reset_opt_reg;
assign reset_conf = reset_conf_reg;
assign conf240p = conf240p_reg;
assign activateHDMIoutput = activateHDMIoutput_reg;
assign hq2x = hq2x_reg;
assign colorspace = colorspace_reg;
assign clock_config_data = clock_config_data_reg;
assign color_config_data = color_config_data_reg;
assign nonBlackPixelReset = nonBlackPixelReset_reg;
assign resetpll = resetpll_reg;

// --- I2C Read
always @(posedge clk) begin
    case(addr)
        // ...
        8'h80: dataOut_reg <= addr_offset;
        8'h81: dataOut_reg <= enable_osd_reg;
        8'h82: dataOut_reg <= highlight_line_reg;
        8'h83: dataOut_reg <= reconf_data_reg;
        8'h84: dataOut_reg <= video_gen_data_reg;
        // controller data, int16
        /*
            15: a
            14: b
            13: x
            12: y
            11: up
            10: down
            09: left
            08: right
        */
        8'h85: dataOut_reg <= controller_data[12:5];
        /*
            07: start
            06: ltrigger
            05: rtrigger
            04: trigger_osd
            03: trigger_default_resolution
        */
        8'h86: dataOut_reg <= { controller_data[4:0], 2'b00, controller_data.valid_packet };
        8'h87: dataOut_reg <= { add_line, line_doubler, is_pal, force_generate, enable_osd_reg, 2'b00, `HQ2X_FLAG };
        // color space data
        8'h88: dataOut_reg <= color_space_explorer[23:16];
        8'h89: dataOut_reg <= color_space_explorer[15:8];
        8'h8A: dataOut_reg <= color_space_explorer[7:0];

        // pll_adv_lockloss_count
        8'hA0: dataOut_reg <= pll_adv_lockloss_count[31:24];
        8'hA1: dataOut_reg <= pll_adv_lockloss_count[23:16];
        8'hA2: dataOut_reg <= pll_adv_lockloss_count[15:8];
        8'hA3: dataOut_reg <= pll_adv_lockloss_count[7:0];

        // hpd_low_count
        8'hA4: dataOut_reg <= hpd_low_count[31:24];
        8'hA5: dataOut_reg <= hpd_low_count[23:16];
        8'hA6: dataOut_reg <= hpd_low_count[15:8];
        8'hA7: dataOut_reg <= hpd_low_count[7:0];

        // pll54_lockloss_count
        8'hA8: dataOut_reg <= pll54_lockloss_count[31:24];
        8'hA9: dataOut_reg <= pll54_lockloss_count[23:16];
        8'hAA: dataOut_reg <= pll54_lockloss_count[15:8];
        8'hAB: dataOut_reg <= pll54_lockloss_count[7:0];

        // pll_hdmi_lockloss_count
        8'hAC: dataOut_reg <= pll_hdmi_lockloss_count[31:24];
        8'hAD: dataOut_reg <= pll_hdmi_lockloss_count[23:16];
        8'hAE: dataOut_reg <= pll_hdmi_lockloss_count[15:8];
        8'hAF: dataOut_reg <= pll_hdmi_lockloss_count[7:0];

        8'hB0: dataOut_reg <= { 2'b0, pinok[21:16] };
        8'hB1: dataOut_reg <= pinok[15:8];
        8'hB2: dataOut_reg <= pinok[7:0];
        8'hB3: dataOut_reg <= timingInfo[23:16];
        8'hB4: dataOut_reg <= timingInfo[15:8];
        8'hB5: dataOut_reg <= timingInfo[7:0];
        8'hB6: dataOut_reg <= rgbData[23:16]; // red
        8'hB7: dataOut_reg <= rgbData[15:8];  // green
        8'hB8: dataOut_reg <= rgbData[7:0];   // blue

        // control_resync_out_count
        8'hB9: dataOut_reg <= control_resync_out_count[31:24];
        8'hBA: dataOut_reg <= control_resync_out_count[23:16];
        8'hBB: dataOut_reg <= control_resync_out_count[15:8];
        8'hBC: dataOut_reg <= control_resync_out_count[7:0];

        // monitor_sense_low_count
        8'hBD: dataOut_reg <= monitor_sense_low_count[31:24];
        8'hBE: dataOut_reg <= monitor_sense_low_count[23:16];
        8'hBF: dataOut_reg <= monitor_sense_low_count[15:8];
        8'hC0: dataOut_reg <= monitor_sense_low_count[7:0];
        8'hC1: dataOut_reg <= testdata[15:8];
        8'hC2: dataOut_reg <= testdata[7:0];

        8'hC3: dataOut_reg <= nonBlackPos1[11:4];
        8'hC4: dataOut_reg <= { nonBlackPos1[3:0], nonBlackPos2[11:8] };
        8'hC5: dataOut_reg <= nonBlackPos2[7:0];

        // clock_config_data
        8'hD0: dataOut_reg <= clock_config_data;
        // color_config_data
        8'hD1: dataOut_reg <= color_config_data;

        8'hE0: dataOut_reg <= { 7'd0, keyboard_data.valid_packet };
        8'hE1: dataOut_reg <= keyboard_data.shiftcode;
        8'hE2: dataOut_reg <= keyboard_data.leds;
        8'hE3: dataOut_reg <= keyboard_data.key1;
        8'hE4: dataOut_reg <= keyboard_data.key2;
        8'hE5: dataOut_reg <= keyboard_data.key3;
        8'hE6: dataOut_reg <= keyboard_data.key4;
        8'hE7: dataOut_reg <= keyboard_data.key5;
        8'hE8: dataOut_reg <= keyboard_data.key6;

        // scanline data
        8'hF5: dataOut_reg <= scanline_reg.intensity[8:1];
        8'hF6: dataOut_reg <= { scanline_reg.intensity[0], scanline_reg.thickness, scanline_reg.oddeven, scanline_reg.active, scanline_reg.dopre, 3'b000 };

        default: dataOut_reg <= 0;
    endcase
end

// --- I2C Write
always @(posedge clk) begin
    if (writeEn == 1'b1) begin
        // address offset for OSD data
        if (addr == 8'h80) begin
            addr_offset <= dataIn[2:0];
        // enable/disable OSD
        end else if (addr == 8'h81) begin
            enable_osd_reg <= dataIn[0];
        // highlight line setting
        end else if (addr == 8'h82) begin 
            highlight_line_reg <= dataIn;
        // output mode reconfiguration
        end else if (addr == 8'h83) begin
            reconf_data_reg <= dataIn;
        // generate video reconfiguration
        end else if (addr == 8'h84) begin
            video_gen_data_reg <= dataIn;
        // 240p offset config
        end else if (addr == 8'h90) begin
            conf240p_reg <= { conf240p_reg[23:8], dataIn };
        end else if (addr == 8'h91) begin
            activateHDMIoutput_reg <= dataIn[0];
        end else if (addr == 8'h92) begin
            hq2x_reg <= dataIn[0];
        end else if (addr == 8'h93) begin
            colorspace_reg <= dataIn[1:0];
        // VGA offset config
        end else if (addr == 8'h94) begin
            conf240p_reg <= { conf240p_reg[23:16], dataIn, conf240p_reg[7:0] };
            nonBlackPixelReset_reg <= 1'b1; // reset counters after setting new offset
        // clock_config_data
        end else if (addr == 8'hD0) begin
            clock_config_data_reg <= dataIn;
        // color_config_data
        end else if (addr == 8'hD1) begin
            color_config_data_reg <= dataIn;
        // reset dreamcast
        end else if (addr == 8'hF0) begin
            reset_dc_reg <= 1'b1;
        // opt reset
        end else if (addr == 8'hF1) begin
            reset_opt_reg <= 1'b1;
        // reset config
        end else if (addr == 8'hF2) begin
            reset_conf_reg <= dataIn;
        // reset non black pixel detect
        end else if (addr == 8'hF3) begin
            nonBlackPixelReset_reg <= 1'b1;
        // reset input PLL
        end else if (addr == 8'hF4) begin
            resetpll_reg <= 1'b1;
        // scanline data
        end else if (addr == 8'hF5) begin
            scanline_reg.intensity[8:1] <= dataIn;
        end else if (addr == 8'hF6) begin
            scanline_reg.intensity[0] <= dataIn[7];
            scanline_reg.thickness <= dataIn[6];
            scanline_reg.oddeven <= dataIn[5];
            scanline_reg.active <= dataIn[4];
            scanline_reg.dopre <= dataIn[3];
        // OSD data
        end else if (addr < 8'h80) begin
            wraddress_reg <= { addr_offset, addr[6:0] };
            wren <= 1'b1;
        end
    end else begin
        wren <= 1'b0;
        reset_dc_reg <= 1'b0;
        reset_opt_reg <= 1'b0;
        nonBlackPixelReset_reg <= 1'b0;
        resetpll_reg <= 1'b0;
    end
end

endmodule


 
