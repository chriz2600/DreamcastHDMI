
`ifndef _960p_inc_
`define _960p_inc_

// 
// 960p
//
`define ASPECT_R                      2'b0
`define PIXEL_CLK              108_000_000

`define HORIZONTAL_PIXELS_PER_LINE   12'd1800
`define HORIZONTAL_PIXELS_VISIBLE    12'd1280
`define HORIZONTAL_SYNC_START        12'd1376
`define HORIZONTAL_SYNC_WIDTH        12'd112
`define HORIZONTAL_SYNC_ON_POLARITY   1'b1

`define VERTICAL_LINES               11'd1000
`define VERTICAL_LINES_INTERLACED    11'd1000
`define VERTICAL_LINES_VISIBLE       11'd960
`define VERTICAL_SYNC_START          11'd961
`define VERTICAL_SYNC_WIDTH          11'd3
`define VERTICAL_SYNC_ON_POLARITY     1'b1

`define HORIZONTAL_OFFSET            10'd0
`define VERTICAL_OFFSET              10'd0
`define PIXEL_FACTOR                  2'd2

`define PLL_CLK0_DIVIDE_BY              11
`define PLL_CLK0_MULTIPLY_BY            16

`define INVERT_VIDEO_CLOCK            1'b0

`define RAM_ADDRESS_BITS                14
`define BUFFER_SIZE                  8'd23
`define TRIGGER_ADDR               15'd320

`endif
