
`ifndef _config_inc_
`define _config_inc_

`define _960p_

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

`endif
