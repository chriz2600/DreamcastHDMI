
`ifndef _1080p_inc_
`define _1080p_inc_

// 
// 1080p
//
// pixel clock and horizontal pixel values
// are half the actual value, because ADV7513's 
// pixel repetition feature is used to create 
// the rx clock
//
`define ASPECT_R                      2'b10
`define PIXEL_CLK                74_250_000

`define HORIZONTAL_PIXELS_PER_LINE   12'd1100
`define HORIZONTAL_PIXELS_VISIBLE    12'd960
`define HORIZONTAL_SYNC_START        12'd1004
`define HORIZONTAL_SYNC_WIDTH        12'd22
`define HORIZONTAL_SYNC_ON_POLARITY   1'b1

`define VERTICAL_LINES               11'd1125
`define VERTICAL_LINES_240P          11'd1125
`define VERTICAL_LINES_VISIBLE       11'd1080
`define VERTICAL_SYNC_START          11'd1084
`define VERTICAL_SYNC_WIDTH          11'd5
`define VERTICAL_SYNC_ON_POLARITY     1'b1

`define HORIZONTAL_OFFSET            10'd160
`define VERTICAL_OFFSET              10'd60
`define VIC_MANUAL                    6'b010000
`define PIXEL_REPETITION

`define PLL_CLK0_DIVIDE_BY               1
`define PLL_CLK0_MULTIPLY_BY             1

`define INVERT_VIDEO_CLOCK            1'b0

`define RAM_ADDRESS_BITS                15
`define BUFFER_SIZE                  8'd34
`define RAM_NUMWORDS                 21760
`define TRIGGER_ADDR (`RAM_NUMWORDS - 320)

`endif
