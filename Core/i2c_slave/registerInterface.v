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
    input DebugData debugData
);

reg [2:0] addr_offset = 3'b000;
reg [7:0] dataOut_reg;
reg [9:0] wraddress_reg;
reg wren;
reg enable_osd_reg = 1'b0;

assign dataOut = dataOut_reg;
assign ram_wraddress = wraddress_reg;
assign ram_dataIn = dataIn;
assign ram_wren = wren;
assign enable_osd = enable_osd_reg;

// --- I2C Read
always @(posedge clk) begin
    case(addr)
        // ...
        8'h80: dataOut_reg <= addr_offset;
        8'h81: dataOut_reg <= enable_osd_reg;
        // general debug data
        8'h90: dataOut_reg <= debugData.pll_errors;
        8'h91: dataOut_reg <= debugData.test;
        8'h92: dataOut_reg <= debugData.frame_counter[7:0];
        8'h93: dataOut_reg <= debugData.frame_counter[9:8];
        8'h94: dataOut_reg <= debugData.pll_status;
        8'h95: dataOut_reg <= debugData.id_check_high;
        8'h96: dataOut_reg <= debugData.id_check_low;
        8'h97: dataOut_reg <= debugData.chip_revision;
        8'h98: dataOut_reg <= debugData.vic_detected;
        8'h99: dataOut_reg <= debugData.vic_to_rx;
        8'h9A: dataOut_reg <= debugData.misc_data;
        8'h9B: dataOut_reg <= debugData.restart_count;
        // CTS debug data
        8'hA0: dataOut_reg <= debugData.cts1_status;
        8'hA1: dataOut_reg <= debugData.cts2_status;
        8'hA2: dataOut_reg <= debugData.cts3_status;
        8'hA3: dataOut_reg <= debugData.max_cts1_status;
        8'hA4: dataOut_reg <= debugData.max_cts2_status;
        8'hA5: dataOut_reg <= debugData.max_cts3_status;
        8'hA6: dataOut_reg <= debugData.summary_cts1_status;
        8'hA7: dataOut_reg <= debugData.summary_cts2_status;
        8'hA8: dataOut_reg <= debugData.summary_cts3_status;
        8'hA9: dataOut_reg <= debugData.summary_summary_cts3_status;
        default: dataOut_reg <= 0;
    endcase
end

// --- I2C Write
always @(posedge clk) begin
    if (writeEn == 1'b1) begin
        if (addr == 8'h80) begin
            addr_offset <= dataIn[2:0];
        end else if (addr == 8'h81) begin
            enable_osd_reg <= dataIn[0];
        end else if (addr < 8'h80) begin
            wraddress_reg <= { addr_offset, addr[6:0] };
            wren <= 1'b1;
        end
    end else begin
        wren <= 1'b0;
    end
end

endmodule


 
