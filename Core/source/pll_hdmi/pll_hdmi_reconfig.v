module pll_hdmi_reconfig (
    input clock,
    input [7:0] address,
    input read_ena,
    input [7:0] data,
    input pll_reconf_busy,

    output q,
    output reconfig,
    output reg trigger_read
);

    reg _read_ena = 0;
    reg q_reg;
    reg q_reg_2;
    reg doReconfig;
    reg doReconfig_2;
    reg doReconfig_3;

    reg [7:0] data_req = 0;

    assign q = q_reg_2;
    assign reconfig = doReconfig_3;

    always @(posedge clock) begin
        _read_ena <= read_ena;

        if (_read_ena && ~read_ena) begin
            doReconfig <= 1;
        end else begin
            doReconfig <= 0;
        end

        if (~pll_reconf_busy && data_req != data) begin
            data_req <= data;
            trigger_read <= 1'b1;
        end else begin
            trigger_read <= 1'b0;
        end

        case (data_req[6:0])
            // RECONF
            7'h00: begin `include "config/148.5_MHz.v" end
            7'h01: begin `include "config/108_MHz.v" end
            7'h02: begin `include "config/27_MHz.v" end
            7'h03: begin `include "config/25.2_MHz.v" end
            7'h10: begin `include "config/144_MHz.v" end
            7'h11: begin `include "config/108_MHz.v" end
            7'h12: begin `include "config/27_MHz.v" end
            7'h13: begin `include "config/25.2_MHz.v" end
            default: begin `include "config/27_MHz.v" end
        endcase

        // delay output, to match ROM based timing
        q_reg_2 <= q_reg;
        doReconfig_2 <= doReconfig;
        doReconfig_3 <= doReconfig_2;
    end
endmodule
