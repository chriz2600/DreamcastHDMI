/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */

module scanline(
    input clock,
    // meta data
    input isDrawArea,
    input isOsdBgArea,
    input isScanline,
    input [8:0] scanline_intensity,
    // ...
    input [23:0] data,
    output reg [23:0] data_out
);
    localparam ONE_TO_ONE = 9'd_256;

    reg [23:0] alpha_data;
    reg [8:0] alpha_alpha;
    wire [23:0] alpha_out;

    alpha_calc ac (
        .clock(clock),
        .data(alpha_data),
        .alpha(alpha_alpha),
        .data_out(alpha_out)
    );

    function [8:0] trunc_osdbg(
        input[16:0] value
    );
        trunc_osdbg = value[16:8];
    endfunction

    reg isOsdBgArea_q, isDrawArea_q, isScanline_q;
    reg [23:0] data_q;

    always @(posedge clock) begin
        data_q <= data;
        { isOsdBgArea_q, isDrawArea_q, isScanline_q } <= { isOsdBgArea, isDrawArea, isScanline };

        case ({ isOsdBgArea_q, isDrawArea_q })
            2'b_01: begin
                alpha_data <= data_q;
                alpha_alpha <= (isScanline_q ? scanline_intensity : ONE_TO_ONE);
            end
            2'b_11: begin
                alpha_data <= data_q;
                alpha_alpha <= ONE_TO_ONE;
            end
            default: begin
                alpha_data <= 24'h00;
                alpha_alpha <= ONE_TO_ONE;
            end
        endcase

        data_out <= alpha_out;
    end
endmodule
