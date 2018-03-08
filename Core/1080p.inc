
`ifndef _1080p_inc_
`define _1080p_inc_

// 
// 1080p
//
`define ASPECT_R                      2'b10
`define PIXEL_CLK              148_500_000

`define HORIZONTAL_PIXELS_PER_LINE   12'd2200
`define HORIZONTAL_PIXELS_VISIBLE    12'd1920
`define HORIZONTAL_SYNC_START        12'd2008
`define HORIZONTAL_SYNC_WIDTH        12'd44
`define HORIZONTAL_SYNC_ON_POLARITY   1'b1

`define VERTICAL_LINES               11'd1125
`define VERTICAL_LINES_INTERLACED    11'd1126
`define VERTICAL_LINES_VISIBLE       11'd1080
`define VERTICAL_SYNC_START          11'd1084
`define VERTICAL_SYNC_WIDTH          11'd5
`define VERTICAL_SYNC_ON_POLARITY     1'b1

`define HORIZONTAL_OFFSET            10'd240
`define VERTICAL_OFFSET              10'd60
`define PIXEL_FACTOR                  2'd2

`define PLL_CLK0_DIVIDE_BY               1
`define PLL_CLK0_MULTIPLY_BY             2

`endif
