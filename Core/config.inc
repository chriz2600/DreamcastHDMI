
`ifndef _config_inc_
`define _config_inc_

`ifdef _VGA_
`include "VGA.inc"
`endif

`ifdef _960p_
`include "960p.inc"
`endif

`ifdef _1080p_
`include "1080p.inc"
`endif

// common variables
`define STARTUP_TIME_IN_MILLIS 200

`define BUFFER_LINE_LENGTH `RAM_ADDRESS_BITS'd640
`define RAM_NUMWORDS (`BUFFER_SIZE * `BUFFER_LINE_LENGTH)

`endif
