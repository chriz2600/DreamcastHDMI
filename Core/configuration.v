module configuration(
    input _480p_active_n,

    output line_doubler,
    output [3:0] clock_config_S
);

`ifdef FORCE_480p
    assign clock_config_S = (4'b1101);
    assign line_doubler = (1'b0);
`elsif FORCE_480i
    assign clock_config_S = (4'b0011);
    assign line_doubler = (1'b1);
`else
    assign clock_config_S = (!_480p_active_n ? 4'b1101 : 4'b0011);
    assign line_doubler = (!_480p_active_n ? 1'b0 : 1'b1);
`endif

endmodule