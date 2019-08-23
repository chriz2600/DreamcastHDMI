
`ifndef _config_inc_
`define _config_inc_

// use this frequency to calculate i2c slave debounce/delay values
`define I2C_BASE_FREQ_FOR_CALCULATIONS 24_000_000

`define OUTPUT_FMT 8'b00000000
`define HQ_TABLE_TYPE wire

// base values for auto detect
`define OFFSET_V_AREA 10'd32
`define OFFSET_DC_CONF 10'd12
`define OFFSET_DC_CONF 10'd12

`ifdef std
    // standard
    `define RAM_WIDTH 15
    `define RAM_MAX_DEPTH 4096
    `define RAM_NUMWORDS 21120
    `define ADV_1080P_REG_3C 8'h_10
    `define HQ2X_FLAG 1'b0
`elsif hq2x
    // HQ2X
    `define RAM_WIDTH 14
    `define RAM_MAX_DEPTH 1024
    `define RAM_NUMWORDS 14720
    `define ADV_1080P_REG_3C 8'h_00
    `define HQ2X_FLAG 1'b1
`endif

`endif
