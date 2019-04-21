/* verilator lint_off UNUSED */

module alpha_calc(
    input clock,
    input [23:0] data,
    input [8:0] alpha,
    output reg [23:0] data_out
);
    function [7:0] trunc_rddata(
        input[15:0] value
    );
        trunc_rddata = value[15:8];
    endfunction

    reg [15:0] r_a, g_a, b_a;

    always @(posedge clock) begin
        r_a <= { 8'b0, data[23:16] } * alpha;
        g_a <= { 8'b0, data[15:8] } * alpha;
        b_a <= { 8'b0, data[7:0] } * alpha;

        data_out <= { trunc_rddata(r_a), trunc_rddata(g_a), trunc_rddata(b_a) };
    end
endmodule

