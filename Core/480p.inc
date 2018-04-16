
`ifndef _480p_inc_
`define _480p_inc_

// 
// 480p
//
`define ASPECT_R                      2'b0
`define PIXEL_CLK               27_000_000

`define HORIZONTAL_PIXELS_PER_LINE   12'd858
`define HORIZONTAL_PIXELS_VISIBLE    12'd720
`define HORIZONTAL_SYNC_START        12'd746
`define HORIZONTAL_SYNC_WIDTH        12'd62
`define HORIZONTAL_SYNC_ON_POLARITY   1'b0

`define VERTICAL_LINES               11'd525
`define VERTICAL_LINES_240P          11'd526
`define VERTICAL_LINES_VISIBLE       11'd480
`define VERTICAL_SYNC_START          11'd489
`define VERTICAL_SYNC_WIDTH          11'd6
`define VERTICAL_SYNC_ON_POLARITY     1'b0

`define HORIZONTAL_OFFSET            10'd0
`define VERTICAL_OFFSET              10'd0

`define ICS664_CLK_DIVIDE_BY            11
`define ICS664_CLK_MULTIPLY_BY           4

`define INVERT_VIDEO_CLOCK            1'b0

`define RAM_ADDRESS_BITS                12
`define BUFFER_SIZE                   8'd1
`define RAM_NUMWORDS                   720
`define TRIGGER_ADDR               15'd360

// special capture for 480p/480i
`define H_CAPTURE_START_P            10'd0
`define H_CAPTURE_END_P            10'd720
`define V_CAPTURE_START_P            10'd0
`define V_CAPTURE_END_P            10'd480

`define H_CAPTURE_START_I            10'd0
`define H_CAPTURE_END_I            10'd720
`define V_CAPTURE_START_I            10'd0
`define V_CAPTURE_END_I            10'd504

`define VERTICAL_LINE_WIDTH            720

`define ICS664_SETTINGS_P          4'b1100
`define ICS664_SETTINGS_I          4'b0010

`define TEXT_OFFSET_COUNTER_X 200
`define TEXT_OFFSET_COUNTER_Y 48

`endif
