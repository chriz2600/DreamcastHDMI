`include "config.inc"

module configuration(
    input _480p_active_n,
    input DCVideoConfig dcVideoConfig,

    output line_doubler,
    output [3:0] clock_config_S
);

`ifdef FORCE_480p
    assign clock_config_S = (dcVideoConfig.ICS644_settings_p);
    assign line_doubler = (1'b0);
`elsif FORCE_480i
    assign clock_config_S = (dcVideoConfig.ICS644_settings_i);
    assign line_doubler = (1'b1);
`else
    assign clock_config_S = (!_480p_active_n ? dcVideoConfig.ICS644_settings_p : dcVideoConfig.ICS644_settings_i);
    assign line_doubler = (!_480p_active_n ? 1'b0 : 1'b1);
`endif

endmodule