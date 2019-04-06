
`ifndef _config_inc_
`define _config_inc_

`define INITIAL_HORIZONTAL_OFFSET 10'd160
`define INITIAL_VERTICAL_OFFSET 10'd60
`define INITIAL_HORIZONTAL_SYNC_ON_POLARITY 1'b1
`define INITIAL_VERTICAL_SYNC_ON_POLARITY 1'b1

// use this frequency to calculate i2c slave debounce/delay values
`define I2C_BASE_FREQ_FOR_CALCULATIONS 24_000_000

`define OUTPUT_FMT 8'b00000000
`define HQ_TABLE_TYPE wire

`endif
