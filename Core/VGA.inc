
`ifndef _VGA_inc_
`define _VGA_inc_

// 
// VGA
//
`define ASPECT_R                    2'b0
`define PIXEL_CLK             25_200_000

`define HORIZONTAL_PIXELS_PER_LINE   800
`define HORIZONTAL_PIXELS_VISIBLE    640
`define HORIZONTAL_SYNC_START        656
`define HORIZONTAL_SYNC_WIDTH         96
`define HORIZONTAL_SYNC_ON_POLARITY 1'b0

`define VERTICAL_LINES               525
`define VERTICAL_LINES_INTERLACED    526
`define VERTICAL_LINES_VISIBLE       480
`define VERTICAL_SYNC_START          490
`define VERTICAL_SYNC_WIDTH            2
`define VERTICAL_SYNC_ON_POLARITY   1'b0

`define HORIZONTAL_START_OFFSET        0
`define PIXEL_FACTOR                   1

`define PLL_CLK0_DIVIDE_BY           165
`define PLL_CLK0_MULTIPLY_BY          56

`endif
