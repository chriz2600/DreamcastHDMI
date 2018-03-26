
`ifndef _VGA_inc_
`define _VGA_inc_

// 
// VGA
//
`define ASPECT_R                      2'b0
`define PIXEL_CLK               25_200_000

`define HORIZONTAL_PIXELS_PER_LINE   12'd800
`define HORIZONTAL_PIXELS_VISIBLE    12'd640
`define HORIZONTAL_SYNC_START        12'd656
`define HORIZONTAL_SYNC_WIDTH        12'd96
`define HORIZONTAL_SYNC_ON_POLARITY   1'b0

`define VERTICAL_LINES               11'd525
`define VERTICAL_LINES_240P          11'd526
`define VERTICAL_LINES_VISIBLE       11'd480
`define VERTICAL_SYNC_START          11'd490
`define VERTICAL_SYNC_WIDTH          11'd2
`define VERTICAL_SYNC_ON_POLARITY     1'b0

`define HORIZONTAL_OFFSET            10'd0
`define VERTICAL_OFFSET              10'd0

`define PLL_CLK0_DIVIDE_BY             165
`define PLL_CLK0_MULTIPLY_BY            56

`define INVERT_VIDEO_CLOCK            1'b0

`define RAM_ADDRESS_BITS                12
`define BUFFER_SIZE                   8'd1
`define RAM_NUMWORDS                   640
`define TRIGGER_ADDR               15'd320

`endif
