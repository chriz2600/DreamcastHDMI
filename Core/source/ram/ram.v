`include "config.inc"

// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on
module ram (
    data,
    rdaddress,
    rdclock,
    wraddress,
    wrclock,
    wren,
    q);

    input	[23:0]  data;
    input	[`RAM_WIDTH-1:0]  rdaddress;
    input	  rdclock;
    input	[`RAM_WIDTH-1:0]  wraddress;
    input	  wrclock;
    input	  wren;
    output	[23:0]  q;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_off
`endif
    tri1	  wrclock;
    tri0	  wren;
`ifndef ALTERA_RESERVED_QIS
// synopsys translate_on
`endif

    wire [23:0] sub_wire0;
    wire [23:0] q = sub_wire0[23:0];

    altsyncram	altsyncram_component (
                .address_a (wraddress),
                .address_b (rdaddress),
                .clock0 (wrclock),
                .clock1 (rdclock),
                .data_a (data),
                .wren_a (wren),
                .q_b (sub_wire0),
                .aclr0 (1'b0),
                .aclr1 (1'b0),
                .addressstall_a (1'b0),
                .addressstall_b (1'b0),
                .byteena_a (1'b1),
                .byteena_b (1'b1),
                .clocken0 (1'b1),
                .clocken1 (1'b1),
                .clocken2 (1'b1),
                .clocken3 (1'b1),
                .data_b ({24{1'b1}}),
                .eccstatus (),
                .q_a (),
                .rden_a (1'b1),
                .rden_b (1'b1),
                .wren_b (1'b0));
    defparam
        altsyncram_component.address_aclr_b = "NONE",
        altsyncram_component.address_reg_b = "CLOCK1",
        altsyncram_component.clock_enable_input_a = "BYPASS",
        altsyncram_component.clock_enable_input_b = "BYPASS",
        altsyncram_component.clock_enable_output_b = "BYPASS",
        altsyncram_component.intended_device_family = "Cyclone 10 LP",
        altsyncram_component.lpm_type = "altsyncram",
        altsyncram_component.maximum_depth = `RAM_MAX_DEPTH,
        altsyncram_component.numwords_a = `RAM_NUMWORDS,
        altsyncram_component.numwords_b = `RAM_NUMWORDS,
        altsyncram_component.operation_mode = "DUAL_PORT",
        altsyncram_component.outdata_aclr_b = "NONE",
        altsyncram_component.outdata_reg_b = "CLOCK1",
        altsyncram_component.power_up_uninitialized = "FALSE",
        altsyncram_component.widthad_a = `RAM_WIDTH,
        altsyncram_component.widthad_b = `RAM_WIDTH,
        altsyncram_component.width_a = 24,
        altsyncram_component.width_b = 24,
        altsyncram_component.width_byteena_a = 1;
endmodule
