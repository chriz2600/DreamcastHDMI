`include "config.inc"

module configuration(
    input clock,
    input DCVideoConfig dcVideoConfig,
    inout _480p_active_n,
    input forceVGAMode,
    input force_generate,
    output reg line_doubler,
    output reg [3:0] clock_config_S,
    output reg config_changed
);
    reg prev_mode = 1'b0;
    reg mode = 1'b0;
    reg _480p_active_n_reg = 1'bz;

    assign _480p_active_n = _480p_active_n_reg;

    always @(posedge clock) begin
        if (prev_mode != mode) begin
            config_changed <= 1'b1;
        end else begin
            config_changed <= 1'b0;
        end

        if (force_generate || forceVGAMode || ~_480p_active_n) begin
            clock_config_S <= dcVideoConfig.ICS644_settings_p;
            line_doubler <= 1'b0;
            mode <= 1'b1;
        end else begin
            clock_config_S <= dcVideoConfig.ICS644_settings_i;
            line_doubler <= 1'b1;
            mode <= 1'b0;
        end

        _480p_active_n_reg <= forceVGAMode ? 1'b0 : 1'bz;
        prev_mode <= mode;
    end

endmodule