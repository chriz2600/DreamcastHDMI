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
    output HDMIVideoConfig hdmiVideoConfig,
    output Scanline scanline,
    output [23:0] conf240p,
    output reset_dc,
    output reset_opt,
    output[7:0] reset_conf,
    input [23:0] pinok,
    input [23:0] timingInfo,
    input [23:0] rgbData,
    input add_line,
    input line_doubler,
    input is_pal,
    input ControllerData controller_data
);

reg [2:0] addr_offset = 3'b000;
reg [7:0] dataOut_reg;
reg [9:0] wraddress_reg;
reg wren;
reg enable_osd_reg = 1'b0;
reg [7:0] highlight_line_reg = 255;
reg [7:0] reconf_data_reg;
reg [7:0] video_gen_data_reg;
reg reset_dc_reg = 1'b0;
reg reset_opt_reg = 1'b0;

`include "../config/hdmi_config.v"

HDMIVideoConfig hdmiVideoConfig_reg;
Scanline scanline_reg = { 9'h100, 1'b0, 1'b0, 1'b0 };
reg [23:0] conf240p_reg = 24'd20;
reg [7:0] reset_conf_reg = 0;

initial begin
    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
end

assign dataOut = dataOut_reg;
assign ram_wraddress = wraddress_reg;
assign ram_dataIn = dataIn;
assign ram_wren = wren;
assign enable_osd = enable_osd_reg;
assign highlight_line = highlight_line_reg;
assign hdmiVideoConfig = hdmiVideoConfig_reg;
assign scanline = scanline_reg;
assign reconf_data = reconf_data_reg;
assign video_gen_data = video_gen_data_reg;
assign reset_dc = reset_dc_reg;
assign reset_opt = reset_opt_reg;
assign reset_conf = reset_conf_reg;
assign conf240p = conf240p_reg;

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
        8'h87: dataOut_reg <= { add_line, line_doubler, is_pal, 5'b000000 };

        // scanline data
        8'h88: dataOut_reg <= scanline_reg.intensity[8:1];
        8'h89: dataOut_reg <= { scanline_reg.intensity[0], scanline_reg.thickness, scanline_reg.oddeven, scanline_reg.active, 4'b0000 };

        8'hB0: dataOut_reg <= { 2'b0, pinok[21:16] };
        8'hB1: dataOut_reg <= pinok[15:8];
        8'hB2: dataOut_reg <= pinok[7:0];
        8'hB3: dataOut_reg <= timingInfo[23:16];
        8'hB4: dataOut_reg <= timingInfo[15:8];
        8'hB5: dataOut_reg <= timingInfo[7:0];
        8'hB6: dataOut_reg <= rgbData[23:16]; // red
        8'hB7: dataOut_reg <= rgbData[15:8];  // green
        8'hB8: dataOut_reg <= rgbData[7:0];   // blue
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
            //wrreq_reg <= 1'b1;
            case (dataIn[3:0])
                // 480p input
                4'h0: begin // 1080p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
                end
                4'h1: begin // 960p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_960P;
                end
                4'h2: begin // 480p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480P;
                end
                4'h3: begin // VGA
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_VGA;
                end
                // 240p input
                4'h8: begin // 240p_1080p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_1080P;
                end
                4'h9: begin // 240p_960p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_960P;
                end
                4'hA: begin // 240p_480p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_480P;
                end
                4'hB: begin // 240p_240p
                    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_VGA;
                end
            endcase
        // generate video reconfiguration
        end else if (addr == 8'h84) begin
            video_gen_data_reg <= dataIn;
        // scanline data
        end else if (addr == 8'h88) begin
            scanline_reg.intensity[8:1] <= dataIn;
        end else if (addr == 8'h89) begin
            scanline_reg.intensity[0] <= dataIn[7];
            scanline_reg.thickness <= dataIn[6];
            scanline_reg.oddeven <= dataIn[5];
            scanline_reg.active <= dataIn[4];
        // 240p config
        end else if (addr == 8'h90) begin
            conf240p_reg <= { 16'd0, dataIn };
        // reset dreamcast
        end else if (addr == 8'hF0) begin
            reset_dc_reg <= 1'b1;
        // opt reset
        end else if (addr == 8'hF1) begin
            reset_opt_reg <= 1'b1;
        // reset config
        end else if (addr == 8'hF2) begin
            reset_conf_reg <= dataIn;
        // OSD data
        end else if (addr < 8'h80) begin
            wraddress_reg <= { addr_offset, addr[6:0] };
            wren <= 1'b1;
        end
    end else begin
        wren <= 1'b0;
        reset_dc_reg <= 1'b0;
        reset_opt_reg <= 1'b0;
    end
end

endmodule


 
