
`ifndef _config_inc_
`define _config_inc_

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

// defaults:
`ifndef ICS664_SETTINGS_P
    `define ICS664_SETTINGS_P    4'b1101
`endif
`ifndef ICS664_SETTINGS_I
    `define ICS664_SETTINGS_I    4'b0011
`endif

`ifndef TEXT_OFFSET_COUNTER_X
    `define TEXT_OFFSET_COUNTER_X       160
`endif
`define TEXT_WIDTH_DIVIDER            8
`define TEXT_RD_ADDR_LOWER_BIT_X      3
`define TEXT_PADDING_X               10
`define TEXT_PADDING_X2               0

`ifdef PIXEL_REPETITION
    `ifndef TEXT_OFFSET_COUNTER_Y
        `define TEXT_OFFSET_COUNTER_Y   100
    `endif
    `define TEXT_HEIGHT_DIVIDER          32
    `define TEXT_RD_ADDR_LOWER_BIT_Y      5
    `define TEXT_CHAR_ADDR_LOWER_BIT_Y    1
    `define TEXT_PADDING_Y               32
    `define TEXT_PADDING_Y2               8
`else
    `ifndef TEXT_OFFSET_COUNTER_Y
        `define TEXT_OFFSET_COUNTER_Y    48
    `endif
    `define TEXT_HEIGHT_DIVIDER          16
    `define TEXT_RD_ADDR_LOWER_BIT_Y      4
    `define TEXT_CHAR_ADDR_LOWER_BIT_Y    0
    `define TEXT_PADDING_Y                6
    `define TEXT_PADDING_Y               32
`endif

// defaults for video2ram
`ifndef H_CAPTURE_START_P
    `define H_CAPTURE_START_P     10'd44
`endif
`ifndef H_CAPTURE_END_P
    `define H_CAPTURE_END_P      10'd684
`endif
`ifndef V_CAPTURE_START_P
    `define V_CAPTURE_START_P      10'd0
`endif
`ifndef V_CAPTURE_END_P
    `define V_CAPTURE_END_P      10'd480
`endif

`ifndef H_CAPTURE_START_I
    `define H_CAPTURE_START_I      10'd1
`endif
`ifndef H_CAPTURE_END_I
    `define H_CAPTURE_END_I      10'd641
`endif
`ifndef V_CAPTURE_START_I
    `define V_CAPTURE_START_I      10'd0
`endif
`ifndef V_CAPTURE_END_I
    `define V_CAPTURE_END_I      10'd504
`endif

`ifndef VERTICAL_LINE_WIDTH
    `define VERTICAL_LINE_WIDTH      640
`endif

// common variables
`define STARTUP_TIME_IN_MILLIS 200

`define BUFFER_LINE_LENGTH `RAM_ADDRESS_BITS'd`VERTICAL_LINE_WIDTH

`endif
