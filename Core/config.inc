
`ifndef _config_inc_
`define _config_inc_

`define H_CAPTURE_START_P     10'd44
`define H_CAPTURE_END_P      10'd684
`define V_CAPTURE_START_P      10'd0
`define V_CAPTURE_END_P      10'd480

`define H_CAPTURE_START_I      10'd1
`define H_CAPTURE_END_I      10'd641
`define V_CAPTURE_START_I      10'd0
`define V_CAPTURE_END_I      10'd504

`define VERTICAL_LINE_WIDTH      640

`ifdef _VGA_
`include "VGA.inc"
`endif

`ifdef _480p_
`include "480p.inc"
`endif

`ifdef _960p_
`include "960p.inc"
`endif

`ifdef _1080p_
`include "1080p.inc"
`endif

// common variables
`define STARTUP_TIME_IN_MILLIS 200

`define BUFFER_LINE_LENGTH `RAM_ADDRESS_BITS'd`VERTICAL_LINE_WIDTH

`endif
