`timescale 1 ns / 1 ns

/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */
/* verilator lint_off UNSIGNED */
/* verilator lint_off DECLFILENAME */
/* verilator lint_off PINCONNECTEMPTY */

`ifdef TEST_BENCH
`include "../../config.inc"
`else
`include "config.inc"
`endif

module ram2video(
    input [23:0] rddata,
    input starttrigger,

    input clock,
    input reset,
    
    input line_doubler,
    input is_interlaced,

    output [`RAM_WIDTH-1:0] rdaddr /*verilator public*/,
    input [7:0] text_rddata,
    output [9:0] text_rdaddr,
    output [23:0] video_out,
    
    output hsync,
    output vsync,
    
    output DrawArea,

    input enable_osd,
    input [7:0] highlight_line,
    input HDMIVideoConfig hdmiVideoConfig /*verilator public*/,
    input Scanline scanline,
    output reg fullcycle
);
    localparam DATA_FETCH_DELAY = 2;

    reg [3:0] _fullcycle;

    reg [7:0] red_reg;
    reg [7:0] green_reg;
    reg [7:0] blue_reg;

    reg hsync_reg_q = 1'b1;
    reg vsync_reg_q = 1'b1;
    reg hsync_reg_q_q = 1'b1;
    reg vsync_reg_q_q = 1'b1;

    reg [11:0] counterX_reg;
    reg [11:0] counterX_reg_q;
    reg [11:0] counterX_reg_q_q;
    reg [11:0] counterX_reg_q_q_q;
    
    reg [11:0] counterX_osd_reg;
    reg [11:0] counterX_osd_reg_q5;

    reg [11:0] counterY_reg;
    reg [11:0] counterY_reg_q/*verilator public*/;
    reg [11:0] counterY_reg_q_q/*verilator public*/;
    reg [11:0] counterY_reg_q_q_q;
    reg [11:0] y_tmp;

    reg [11:0] vert_lines;
    reg [11:0] sync_start;
    reg [11:0] sync_pixel_offset;
    reg [3:0] shift_y;

    reg [7:0] currentLine_reg;
    reg [7:0] currentLine_reg_q;

    reg [3:0] charPixelRow_reg;

    reg [9:0] ram_addrX_reg/*verilator public*/;
    reg [`RAM_WIDTH-1:0] ram_addrY_reg;
    reg [3:0] pxl_rep_c_x;
    reg [3:0] pxl_rep_c_y;

    reg trigger = 1'b0;

    reg [7:0] char_data_reg;
    reg [7:0] char_data_reg_q;
    reg [9:0] text_rdaddr_x;
    reg [9:0] text_rdaddr_y;

    reg isScanline = 0;

    reg [23:0] _d_video_out;
    reg _d_hsync;
    reg _d_vsync;
    reg _d_DrawArea;

    reg [`RAM_WIDTH-1:0] d_rdaddr;
    reg [23:0] d_video_out;
    reg d_hsync;
    reg d_vsync;
    reg d_DrawArea;
    reg state;

    wire [11:0] char_addr;
    wire [7:0] char_data;
    char_rom char_rom_inst(
        .address(char_addr),
        .clock(clock),
        .q(char_data)
    );

    delayline #(
        .CYCLES(4),
        .WIDTH(12)
    ) osd_delay (
        .clock(clock),
        .in(counterX_osd_reg),
        .out(counterX_osd_reg_q5)
    );

    `define IsOsdBgArea(x, y)  ( \
        enable_osd \
        && x >= hdmiVideoConfig.osd_bg_offset_x_start \
        && x < hdmiVideoConfig.osd_bg_offset_x_end \
        && y >= hdmiVideoConfig.osd_bg_offset_y_start \
        && y < hdmiVideoConfig.osd_bg_offset_y_end)

    `define IsOsdTextArea(x, y)  ( \
        enable_osd \
        && x >= hdmiVideoConfig.osd_text_x_start \
        && x < hdmiVideoConfig.osd_text_x_end \
        && y >= hdmiVideoConfig.osd_text_y_start \
        && y < hdmiVideoConfig.osd_text_y_end)

    `define IsDrawAreaHDMI(x, y)   (x >= 0 && x < hdmiVideoConfig.horizontal_pixels_visible \
                                 && y >= 0 && y < hdmiVideoConfig.vertical_lines_visible)

    `define IsDrawAreaVGA(x, y)   (x >= hdmiVideoConfig.horizontal_capture_start \
                                && x < hdmiVideoConfig.horizontal_capture_end \
                                && y >= hdmiVideoConfig.vertical_capture_start \
                                && y < hdmiVideoConfig.vertical_capture_end)

    `define GetAddr(x, y) (`IsDrawAreaVGA(x, y) ? ram_addrY_reg + ram_addrX_reg : 14'd0)

    function [7:0] truncate_rddata(
        input[15:0] value
    );
        truncate_rddata = value[15:8];
    endfunction

    function [8:0] truncate_osdbg(
        input[16:0] value
    );
        truncate_osdbg = value[16:8];
    endfunction

    function [8:0] osdaddr(
        input[11:0] value
    );
        osdaddr = value[11:3];
    endfunction

    `define GetRdData(data, a) ({ \
        truncate_rddata({ 8'b0, data[23:16] } * a), \
        truncate_rddata({ 8'b0, data[15:8] } * a), \
        truncate_rddata({ 8'b0, data[7:0] } * a) \
    })

    `define VerticalLines (vert_lines)
    `define VerticalSyncStart (sync_start)
    `define VerticalSyncPixelOffset (sync_pixel_offset)

    output_data out_dat(
        .clock(clock),
        .isDrawAreaVGA(`IsDrawAreaVGA(counterX_reg_q_q_q, counterY_reg_q_q_q)),
        .isOsdTextArea(`IsOsdTextArea(counterX_reg_q_q_q, counterY_reg_q_q_q)),
        .isCharPixel(char_data[7-counterX_osd_reg_q5[2:0]] ^ (currentLine_reg_q == highlight_line)),
        .isOsdBgArea(`IsOsdBgArea(counterX_reg_q_q_q, counterY_reg_q_q_q)),
        .isScanline(isScanline),
        .scanline_intensity(scanline.intensity),
        .data(rddata),
        .data_out(_d_video_out)
    );

    always @(posedge clock or posedge reset) begin
        if (reset) begin
            trigger <= 1'b0;
            fullcycle <= 0;
            _fullcycle <= 0;
        end else if (!trigger) begin
            // wait for trigger to start
            if (starttrigger) begin
                trigger <= 1'b1;
                counterX_reg <= hdmiVideoConfig.horizontal_offset;
                counterY_reg <= hdmiVideoConfig.vertical_offset;
                hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
                vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity;
                ram_addrX_reg <= 0;
                ram_addrY_reg <= 0;
                pxl_rep_c_x <= 0;
                pxl_rep_c_y <= 0;
                state <= 0;
            end
        end else begin
            //////////////////////////////////////////////////////////////////////
            // set vertical value
            case (state)
                0: begin
                    vert_lines <= hdmiVideoConfig.vertical_lines_1;
                    sync_start <= hdmiVideoConfig.vertical_sync_start_1;
                    sync_pixel_offset <= hdmiVideoConfig.vertical_sync_pixel_offset_1;
                    shift_y <= is_interlaced && hdmiVideoConfig.pxl_rep_on ? 0 : 0;
                end
                1: begin
                    vert_lines <= hdmiVideoConfig.vertical_lines_2;
                    sync_start <= hdmiVideoConfig.vertical_sync_start_2;
                    sync_pixel_offset <= hdmiVideoConfig.vertical_sync_pixel_offset_2;
                    shift_y <= is_interlaced && hdmiVideoConfig.pxl_rep_on ? 4'd_2 : 0;
                end
            endcase

            //////////////////////////////////////////////////////////////////////
            // trigger is set, output data
            if (counterX_reg < hdmiVideoConfig.horizontal_pixels_per_line - 1) begin
                counterX_reg <= counterX_reg + 1'b1;

                if (counterX_reg >= hdmiVideoConfig.horizontal_offset) begin
                    if (hdmiVideoConfig.pxl_rep_on) begin
                        if (pxl_rep_c_x == hdmiVideoConfig.pxl_rep_h - 1) begin
                            ram_addrX_reg <= ram_addrX_reg + hdmiVideoConfig.pxl_rep_addr_inr_h;
                            pxl_rep_c_x <= 0;
                        end else begin
                            pxl_rep_c_x <= pxl_rep_c_x + 1'b1;
                        end
                    end else begin
                        ram_addrX_reg <= ram_addrX_reg + 1'b1;
                    end
                end else begin
                    ram_addrX_reg <= 0;
                    pxl_rep_c_x <= 0;
                end
            end else begin
                counterX_reg <= 0;
                ram_addrX_reg <= 0;
                pxl_rep_c_x <= 0;

                if (counterY_reg < `VerticalLines - 1) begin
                    counterY_reg <= counterY_reg + 1'b1;

                    if (counterY_reg >= hdmiVideoConfig.vertical_offset + shift_y) begin
                        if (ram_addrY_reg < hdmiVideoConfig.ram_numwords - hdmiVideoConfig.buffer_line_length) begin
                            if (hdmiVideoConfig.pxl_rep_on) begin
                                if (pxl_rep_c_y == ((line_doubler ? hdmiVideoConfig.pxl_rep_v_i : hdmiVideoConfig.pxl_rep_v) - 1)) begin
                                    ram_addrY_reg <= ram_addrY_reg + hdmiVideoConfig.buffer_line_length;
                                    pxl_rep_c_y <= 0;
                                end else begin
                                    pxl_rep_c_y <= pxl_rep_c_y + 1'b1;
                                end
                            end else begin
                                if (hdmiVideoConfig.line_doubling) begin
                                    if (counterY_reg[0] && (!line_doubler || counterY_reg[1])) begin
                                        ram_addrY_reg <= ram_addrY_reg + hdmiVideoConfig.buffer_line_length;
                                    end
                                end else begin
                                    if (!line_doubler || counterY_reg[0]) begin
                                        ram_addrY_reg <= ram_addrY_reg + hdmiVideoConfig.buffer_line_length;
                                    end
                                end
                            end
                        end else begin
                            if (hdmiVideoConfig.pxl_rep_on) begin
                                if (pxl_rep_c_y == ((line_doubler ? hdmiVideoConfig.pxl_rep_v_i : hdmiVideoConfig.pxl_rep_v) - 1)) begin
                                    ram_addrY_reg <= 0;
                                    pxl_rep_c_y <= 0;
                                end else begin
                                    pxl_rep_c_y <= pxl_rep_c_y + 1'b1;
                                end
                            end else begin
                                if (hdmiVideoConfig.line_doubling) begin
                                    if (counterY_reg[0] && (!line_doubler || counterY_reg[1])) begin
                                        ram_addrY_reg <= 0;
                                    end
                                end else begin
                                    if (!line_doubler || counterY_reg[0]) begin
                                        ram_addrY_reg <= 0;
                                    end
                                end
                            end
                        end
                    end
                end else begin
                    counterY_reg <= 0;
                    ram_addrY_reg <= 0;
                    pxl_rep_c_y <= 0;
                    state <= ~state;
                end
            end

            //////////////////////////////////////////////////////////////////////
            // generate output hsync
            if (counterX_reg_q >= hdmiVideoConfig.horizontal_sync_start && counterX_reg_q < hdmiVideoConfig.horizontal_sync_start + hdmiVideoConfig.horizontal_sync_width) begin
                hsync_reg_q <= hdmiVideoConfig.horizontal_sync_on_polarity;
            end else begin
                hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
            end

            //////////////////////////////////////////////////////////////////////
            // generate output vsync
            if (counterY_reg_q >= `VerticalSyncStart && counterY_reg_q < `VerticalSyncStart + hdmiVideoConfig.vertical_sync_width + 1) begin
                if ((counterY_reg_q == `VerticalSyncStart && counterX_reg_q < `VerticalSyncPixelOffset) 
                    || (counterY_reg_q == `VerticalSyncStart + hdmiVideoConfig.vertical_sync_width && counterX_reg_q >= `VerticalSyncPixelOffset)) begin
                    vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity; // OFF
                end else begin
                    vsync_reg_q <= hdmiVideoConfig.vertical_sync_on_polarity; // ON
                end
            end else begin
                vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity; // OFF
            end

            if (vsync_reg_q == hdmiVideoConfig.vertical_sync_on_polarity) begin
                _fullcycle <= _fullcycle + 1'b1;
            end

            //////////////////////////////////////////////////////////////////////
            // delay queue
            counterX_reg_q <= counterX_reg;
            counterX_reg_q_q <= counterX_reg_q;
            counterX_reg_q_q_q <= counterX_reg_q_q;

            counterY_reg_q <= counterY_reg;
            counterY_reg_q_q <= counterY_reg_q;
            counterY_reg_q_q_q <= counterY_reg_q_q;

            hsync_reg_q_q <= hsync_reg_q;
            vsync_reg_q_q <= vsync_reg_q;

            //////////////////////////////////////////////////////////////////////
            // OSD TEXT
            y_tmp <= counterY_reg - hdmiVideoConfig.osd_text_y_start;
            if (hdmiVideoConfig.line_doubling) begin
                currentLine_reg <= (hdmiVideoConfig.interlaceOSD ? y_tmp[11:4] : y_tmp[11:5]);
                charPixelRow_reg <= (hdmiVideoConfig.interlaceOSD ? y_tmp[3:1] << 1'b1 : y_tmp[4:1]) + (hdmiVideoConfig.interlaceOSD && state ? 1'b1 : 1'b0);
            end else begin
                currentLine_reg <= (hdmiVideoConfig.interlaceOSD ? y_tmp[10:3] : y_tmp[11:4]);
                charPixelRow_reg <= (hdmiVideoConfig.interlaceOSD ? y_tmp[2:0] << 1'b1 : y_tmp[3:0]) + (hdmiVideoConfig.interlaceOSD && state ? 1'b1 : 1'b0);
            end

            text_rdaddr_y <= currentLine_reg * 10'd40;

            if (counterX_reg + DATA_FETCH_DELAY >= hdmiVideoConfig.osd_text_x_start) begin
                text_rdaddr_x <= osdaddr(counterX_osd_reg);
            end else begin
                text_rdaddr_x <= 0;
            end

            if (counterX_reg + DATA_FETCH_DELAY + 1 == hdmiVideoConfig.osd_text_x_start) begin
                counterX_osd_reg <= 0;
            end else begin
                if (hdmiVideoConfig.pixel_repetition) begin
                    counterX_osd_reg <= counterX_osd_reg + counterX_reg[0];
                end else begin
                    counterX_osd_reg <= counterX_osd_reg + 1'b1;
                end
            end

            currentLine_reg_q <= currentLine_reg;

            //////////////////////////////////////////////////////////////////////
            // SCANLINES
            if (scanline.active) begin
                if (hdmiVideoConfig.line_doubling && scanline.dopre) begin
                    isScanline <= counterY_reg[2:1] >> scanline.thickness ^ scanline.oddeven;
                end else begin
                    isScanline <= counterY_reg[1:0] >> scanline.thickness ^ scanline.oddeven;
                end
            end else begin
                isScanline <= 1'b0;
            end

            //////////////////////////////////////////////////////////////////////
            // OUTPUT
            d_rdaddr <= `GetAddr(counterX_reg, counterY_reg);
            fullcycle <= fullcycle || _fullcycle == 4'b1111;

            _d_DrawArea <= `IsDrawAreaHDMI(counterX_reg_q_q_q, counterY_reg_q_q_q);
            _d_hsync <= hsync_reg_q_q;
            _d_vsync <= vsync_reg_q_q;
        end
    end

    delayline #(
        .CYCLES(8),
        .WIDTH(3)
    ) vout_delay1 (
        .clock(clock),
        .in({ _d_DrawArea, _d_hsync, _d_vsync }),
        .out({ d_DrawArea, d_hsync, d_vsync })
    );

    delayline #(
        .CYCLES(4),
        .WIDTH(24)
    ) vout_delay2 (
        .clock(clock),
        .in({ _d_video_out }),
        .out({ d_video_out })
    );

    assign text_rdaddr = text_rdaddr_x + text_rdaddr_y;
    assign char_addr = (text_rddata << 4) + charPixelRow_reg;
    assign rdaddr = d_rdaddr;
    assign video_out = d_video_out;
    assign DrawArea = d_DrawArea;
    assign hsync = d_hsync;
    assign vsync = d_vsync;

endmodule

