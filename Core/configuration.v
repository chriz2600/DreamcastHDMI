`include "config.inc"

module configuration(
    input DCVideoConfig dcVideoConfig,
    input _480p_active_n,
    output line_doubler,
    output [3:0] clock_config_S
);

    assign clock_config_S = !_480p_active_n ? dcVideoConfig.ICS644_settings_p : dcVideoConfig.ICS644_settings_i;
    assign line_doubler = !_480p_active_n ? 1'b0 : 1'b1;

endmodule