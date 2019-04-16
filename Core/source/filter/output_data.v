/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */

module output_data(
    input clock,
    // meta data
    input isDrawAreaVGA,
    input isOsdTextArea,
    input isCharPixel,
    input isOsdBgArea,
    input isScanline,
    input [8:0] scanline_intensity,
    // ...
    input [23:0] data,
    output reg [23:0] data_out
);
    localparam ONE_TO_ONE = 9'd_256;
    `ifdef OSD_BACKGROUND_ALPHA
        localparam OSD_BACKGROUND_ALPHA = `OSD_BACKGROUND_ALPHA;
    `else
        localparam OSD_BACKGROUND_ALPHA = 9'd_64;
    `endif

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

    reg [8:0] osd_alpha;
    reg isCharPixel_q, isOsdTextArea_q, isOsdBgArea_q, isDrawAreaVGA_q, isScanline_q;
    reg [23:0] data_q;

    always @(posedge clock) begin
        data_q <= data;
        osd_alpha <= OSD_BACKGROUND_ALPHA * scanline_intensity;
        { isCharPixel_q, isOsdTextArea_q, isOsdBgArea_q, isDrawAreaVGA_q, isScanline_q } <= { isCharPixel, isOsdTextArea, isOsdBgArea, isDrawAreaVGA, isScanline };

        case ({ isOsdTextArea_q, isOsdBgArea_q, isDrawAreaVGA_q })
            3'b_001: begin
                alpha_data <= data_q;
                alpha_alpha <= (isScanline_q ? scanline_intensity : ONE_TO_ONE);
            end
            3'b_011: begin
                alpha_data <= data_q;
                alpha_alpha <= (isScanline_q ? trunc_osdbg(osd_alpha) : OSD_BACKGROUND_ALPHA);
            end
            3'b_111: begin
                if (isCharPixel_q) begin
                    alpha_data <= 24'hFFFFFF;
                    alpha_alpha <= ONE_TO_ONE;
                end else begin
                    alpha_data <= data_q;
                    alpha_alpha <= (isScanline_q ? trunc_osdbg(osd_alpha) : OSD_BACKGROUND_ALPHA);
                end
            end
            default: begin
                alpha_data <= 24'h00;
                alpha_alpha <= ONE_TO_ONE;
            end
        endcase

        data_out <= alpha_out;
    end
endmodule
