
`ifndef _960p_inc_
`define _960p_inc_

// 
// 960p
//
`define ASPECT_R 2'b0
`define PIXEL_CLK 108_000_000

`define HORIZONTAL_PIXELS_PER_LINE  1800
`define HORIZONTAL_PIXELS_VISIBLE   1280
`define HORIZONTAL_SYNC_START       1376
`define HORIZONTAL_SYNC_WIDTH        112
`define HORIZONTAL_SYNC_ON_POLARITY 1'b1

`define VERTICAL_LINES              1000
`define VERTICAL_LINES_INTERLACED   1000
`define VERTICAL_LINES_VISIBLE       960
`define VERTICAL_SYNC_START          961
`define VERTICAL_SYNC_WIDTH            3
`define VERTICAL_SYNC_ON_POLARITY   1'b1

`define HORIZONTAL_START_OFFSET        0
`define PIXEL_FACTOR                   2

`define PLL_CLK0_DIVIDE_BY				  11
`define PLL_CLK0_MULTIPLY_BY			  16

`endif
