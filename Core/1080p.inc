
`ifndef _1080p_inc_
`define _1080p_inc_

// 
// 1080p
//
`define ASPECT_R                   2'b10
`define PIXEL_CLK            148_500_000

`define HORIZONTAL_PIXELS_PER_LINE  2200
`define HORIZONTAL_PIXELS_VISIBLE   1920
`define HORIZONTAL_SYNC_START       2008
`define HORIZONTAL_SYNC_WIDTH         44
`define HORIZONTAL_SYNC_ON_POLARITY 1'b1

`define VERTICAL_LINES              1125
`define VERTICAL_LINES_INTERLACED   1126
`define VERTICAL_LINES_VISIBLE      1080
`define VERTICAL_SYNC_START         1084
`define VERTICAL_SYNC_WIDTH            5
`define VERTICAL_SYNC_ON_POLARITY   1'b1

`define HORIZONTAL_OFFSET            240
`define VERTICAL_OFFSET               60
`define PIXEL_FACTOR                   2

`define PLL_CLK0_DIVIDE_BY             1
`define PLL_CLK0_MULTIPLY_BY           2

`endif
