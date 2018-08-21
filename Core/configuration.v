`include "config.inc"

module configuration(
    input clock,
    input DCVideoConfig dcVideoConfig,
    inout _480p_active_n,
    input forceVGAMode,
    output reg line_doubler,
    output reg [3:0] clock_config_S
);

    reg _480p_active_n_reg = 1'bz;

    assign _480p_active_n = _480p_active_n_reg;

    always @(posedge clock) begin
        if (forceVGAMode || ~_480p_active_n) begin
            clock_config_S <= dcVideoConfig.ICS644_settings_p;
            line_doubler <= 1'b0;
            _480p_active_n_reg <= 1'b0;
        end else begin
            clock_config_S <= dcVideoConfig.ICS644_settings_i;
            line_doubler <= 1'b1;
            _480p_active_n_reg <= 1'bz;
        end
    end

endmodule