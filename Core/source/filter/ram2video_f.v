/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */

`include "config.inc"

module ram2video_f(
    input clock,
    input reset,
    input starttrigger,
    input hq2x,
    output reg fullcycle,
    input Scanline scanline,

    output [`RAM_WIDTH-1:0] rdaddr /*verilator public*/,
    input [23:0] rddata,

    output [9:0] text_rdaddr /*verilator public*/,
    input [7:0] text_rddata,
    input enable_osd,
    input [7:0] highlight_line,

    //input line_doubler,
    input HDMIVideoConfig hdmiVideoConfig /*verilator public*/,

    // video output
    output [23:0] video_out,
    output hsync,
    output vsync,
    output DrawArea
);
    localparam PXL_REP_H_HQ2X = 4;
    localparam SKIP_LINES = 12'd3;
    localparam DATA_DELAY_START = 4;
    localparam DATA_DELAY_END = 2;

    localparam OSD_PXL_REP_H_HQ2X = 32;
    localparam OSD_FONT_HEIGHT = 16;

    reg [23:0] inputpixel;
    wire [23:0] outpixel;
    wire [23:0] outpixel2;
    Hq2x_optimized hq2x_inst (
        .clk(clock),
        .ce_x4(1'b1),
        .inputpixel(inputpixel),
        .mono(1'b0),
        .disable_hq2x(~hq2x),
        .reset_frame(reset_frame_out),
        .reset_line(reset_line_out),
        .read_y(read_y),
        .hblank(hblank),
        .outpixel(outpixel)
    );

    delayline #(
        .CYCLES(3),
        .WIDTH(12)
    ) osd_delay (
        .clock(clock),
        .in(counterX_osd_reg),
        .out(counterX_osd_reg_q)
    );

    delayline #(
        .CYCLES(7),
        .WIDTH(2)
    ) rl_rf_delay (
        .clock(clock),
        .in({ next_reset_frame, next_reset_line }),
        .out({ reset_frame_out, reset_line_out })
    );

    localparam OSD_BG_OFFSET_X_START = 12'd150;
    localparam OSD_BG_OFFSET_X_END = 12'd490;
    localparam OSD_BG_OFFSET_Y_START = 11'd38;
    localparam OSD_BG_OFFSET_Y_END = 11'd447;

    localparam OSD_TEXT_X_START = 12'd160;
    localparam OSD_TEXT_X_END = 12'd480;
    localparam OSD_TEXT_Y_START = 11'd48;
    localparam OSD_TEXT_Y_END = 11'd432;

    output_data out_dat(
        .clock(clock),
        .isDrawAreaVGA(counterX_reg_vga < 640 && counterY_reg_vga < 480),
        .isOsdBgArea(
               enable_osd
            && counterX_reg_vga >= OSD_BG_OFFSET_X_START && counterX_reg_vga < OSD_BG_OFFSET_X_END
            && counterY_reg_vga >= OSD_BG_OFFSET_Y_START && counterY_reg_vga < OSD_BG_OFFSET_Y_END
        ),
        .isOsdTextArea(
               enable_osd
            && counterX_reg_vga >= OSD_TEXT_X_START && counterX_reg_vga < OSD_TEXT_X_END
            && counterY_reg_vga >= OSD_TEXT_Y_START && counterY_reg_vga < OSD_TEXT_Y_END
        ),
        .isCharPixel(char_data[7-counterX_osd_reg_q[2:0]] ^ (counterY_osd_reg == highlight_line)),
        .isScanline(/*isScanline*/0),
        .scanline_intensity(scanline.intensity),
        .data({ rddata[7:0], rddata[15:8], rddata[23:16] }),
        .data_out(inputpixel)
    );

    `define IsDrawAreaHDMI_f(x, y)   (x >= 0 && x < hdmiVideoConfig.horizontal_pixels_visible \
                                   && y >= 0 && y < hdmiVideoConfig.vertical_lines_visible)

    `define IsDrawAreaVGA_f(x, y)   (x >= hdmiVideoConfig.horizontal_capture_start \
                                  && x < hdmiVideoConfig.horizontal_capture_end \
                                  && y >= hdmiVideoConfig.vertical_capture_start \
                                  && y < hdmiVideoConfig.vertical_capture_end)

    `define IsOsdBgArea_f(x, y)  ( \
        enable_osd \
        && x >= hdmiVideoConfig.osd_bg_offset_x_start \
        && x < hdmiVideoConfig.osd_bg_offset_x_end \
        && y >= hdmiVideoConfig.osd_bg_offset_y_start \
        && y < hdmiVideoConfig.osd_bg_offset_y_end)

    scanline scnl(
        .clock(clock),
        .isDrawArea(`IsDrawAreaHDMI_f(counterX_reg_q_q_q_q, counterY_shift_q_q_q_q)),
        .isOsdBgArea(`IsOsdBgArea_f(counterX_reg_q_q_q_q, counterY_shift_q_q_q_q)),
        .isScanline(isScanline),
        .scanline_intensity(scanline.intensity),
        .data({ outpixel[7:0], outpixel[15:8], outpixel[23:16] }),
        .data_out(outpixel2)
    );

    wire [11:0] char_addr;
    wire [7:0] char_data;
    char_rom char_rom_inst(
        .address(char_addr),
        .clock(clock),
        .q(char_data)
    );

    /* verilator lint_off UNSIGNED */

    `define IsHBlank(x, y) (( \
           x >= hdmiVideoConfig.horizontal_hq2x_start \
        && x < hdmiVideoConfig.horizontal_hq2x_end \
    ) ^ hdmiVideoConfig.is_hq2x_display_area)

    `define ResetReadY(y) (y == `VerticalLines_f - 1)
    `define AdvanceReadY(x, y) (x == hdmiVideoConfig.horizontal_capture_end + DATA_DELAY_END && (y >= hdmiVideoConfig.vertical_capture_start))

    `define GetAddr_f(x, y) (next_reset_frame | next_reset_line ? 14'b0 : ram_addrY_reg_hq2x + { 4'b0, ram_addrX_reg_hq2x })
    `define GetData_f(x, y) (`IsDrawAreaVGA_f(x, y) ? outpixel2 : 24'h00)

    reg [9:0] ram_addrX_reg_hq2x /*verilator public*/;
    reg [`RAM_WIDTH-1:0] ram_addrY_reg_hq2x /*verilator public*/;

    `define VerticalLines_f (vert_lines)
    `define VerticalSyncStart_f (sync_start)
    `define VerticalSyncPixelOffset_f (sync_pixel_offset)

    `define StartInputCounter(x, y) (x == hdmiVideoConfig.horizontal_capture_start && y == hdmiVideoConfig.vertical_capture_start && y[0] == 0)
    `define StopInputCounter(x, y) (y == hdmiVideoConfig.vertical_capture_end - 1)
    `define AdvanceInputCounter(x, y) (x == hdmiVideoConfig.horizontal_capture_start && y >= hdmiVideoConfig.vertical_capture_start && y[0] == 0)

    `define StartOSDInputCounter(x, y) (\
           x == hdmiVideoConfig.horizontal_capture_start + (OSD_TEXT_X_START << 2) \
        && y == hdmiVideoConfig.vertical_capture_start + (OSD_TEXT_Y_START << 1) \
        && y[0] == 0 \
    )
    `define StopOSDInputCounter(x, y) (y == (OSD_TEXT_Y_END << 1) - 1)
    `define AdvanceOSDInputCounter(x, y) (\
           x == hdmiVideoConfig.horizontal_capture_start + (OSD_TEXT_X_START << 2) \
        && y >= hdmiVideoConfig.vertical_capture_start + (OSD_TEXT_Y_START << 1) \
        && y[0] == 0 \
    )

    reg trigger /*verilator public*/;
    reg state /*verilator public*/;
    reg [11:0] counterX_reg /*verilator public*/;
    reg [11:0] counterX_reg_q /*verilator public*/;
    reg [11:0] counterX_reg_q_q /*verilator public*/;
    reg [11:0] counterX_reg_q_q_q /*verilator public*/;
    reg [11:0] counterX_reg_q_q_q_q /*verilator public*/;
    reg [11:0] counterX_reg_out /*verilator public*/;
    reg [11:0] counterY_reg /*verilator public*/;
    reg [11:0] counterY_reg_q /*verilator public*/;
    reg [11:0] counterY_shift_q /*verilator public*/;
    reg [11:0] counterY_shift_q_q /*verilator public*/;
    reg [11:0] counterY_shift_q_q_q /*verilator public*/;
    reg [11:0] counterY_shift_q_q_q_q /*verilator public*/;
    reg [11:0] counterY_shift_out /*verilator public*/;

    reg [11:0] counterX_reg_vga;
    reg [11:0] counterY_reg_vga;

    reg [11:0] counterX_osd_reg;
    reg [11:0] counterX_osd_reg_q;
    reg [11:0] counterY_osd_reg;

    reg [11:0] vert_lines;
    reg [11:0] sync_start;
    reg [11:0] sync_pixel_offset;

    reg [7:0] currentLine_reg;
    reg [7:0] currentLine_reg_q;
    reg [3:0] charPixelRow_reg;
    reg [7:0] char_data_reg;
    reg [7:0] char_data_reg_q;
    reg [9:0] text_rdaddr_x;
    reg [10:0] text_rdaddr_y;

    reg hsync_reg_q /*verilator public*/;
    reg vsync_reg_q /*verilator public*/;
    reg hsync_reg_q_q /*verilator public*/;
    reg vsync_reg_q_q /*verilator public*/;
    reg hsync_reg_q_q_q /*verilator public*/;
    reg vsync_reg_q_q_q /*verilator public*/;
    reg hsync_reg_out /*verilator public*/;
    reg vsync_reg_out /*verilator public*/;

    /* verilator lint_off UNUSED */
    reg [`RAM_WIDTH-1:0] d_rdaddr /*verilator public*/;

    reg [23:0] _d_video_out;
    reg _d_hsync;
    reg _d_vsync;
    reg _d_DrawArea;

    reg [23:0] d_video_out;
    reg d_hsync;
    reg d_vsync;
    reg d_DrawArea;

    reg [3:0] pxl_rep_c_x_hq2x;
    reg [5:0] pxl_rep_c_x_osd;
    reg [3:0] pxl_rep_c_x_osd_pxl;

    reg next_reset_line /*verilator public*/;
    reg next_reset_frame /*verilator public*/;
    reg reset_frame_out, reset_line_out;

    reg hblank /*verilator public*/;
    reg [1:0] read_y /*verilator public*/;
    reg [3:0] _fullcycle;
    reg isScanline = 0;

    always @(posedge clock or posedge reset) begin
        if (reset) begin
            trigger <= 1'b0;
            fullcycle <= 0;
            _fullcycle <= 0;
        end else if (!trigger) begin
            // wait for trigger to start
            if (starttrigger) begin
                trigger <= 1'b1;
                counterX_reg <= hdmiVideoConfig.horizontal_capture_start;
                counterY_reg <= hdmiVideoConfig.vertical_capture_start;
                counterY_shift_q <= hdmiVideoConfig.vertical_capture_start - SKIP_LINES;
                hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
                vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity;
                ram_addrX_reg_hq2x <= 0;
                ram_addrY_reg_hq2x <= 0;
                next_reset_line <= 1;
                pxl_rep_c_x_hq2x <= 0;
                pxl_rep_c_x_osd <= 0;
                pxl_rep_c_x_osd_pxl <= 0;
                state <= 0;
                read_y <= 2'd1;
            end
        end else begin
            //////////////////////////////////////////////////////////////////////
            // trigger is set, output data
            //////////////////////////////////////////////////////////////////////
            //////////////////////////////////////////////////////////////////////
            // set vertical value
            case (state)
                0: begin
                    vert_lines <= hdmiVideoConfig.vertical_lines_1;
                    sync_start <= hdmiVideoConfig.vertical_sync_start_1;
                    sync_pixel_offset <= hdmiVideoConfig.vertical_sync_pixel_offset_1;
                end
                1: begin
                    vert_lines <= hdmiVideoConfig.vertical_lines_2;
                    sync_start <= hdmiVideoConfig.vertical_sync_start_2;
                    sync_pixel_offset <= hdmiVideoConfig.vertical_sync_pixel_offset_2;
                end
            endcase

            //////////////////////////////////////////////////////////////////////
            // generate read_y and hblank signals
            if (`ResetReadY(counterY_reg_q)) begin
                read_y <= 2'd1;
            end else if (`AdvanceReadY(counterX_reg_q, counterY_shift_q)) begin
                read_y <= read_y + 1'b1;
            end
            hblank <= `IsHBlank(counterX_reg_q, counterY_reg_q);

            //////////////////////////////////////////////////////////////////////
            // generate ram read
            if (`AdvanceInputCounter(counterX_reg_q, counterY_reg_q)) begin
                ram_addrX_reg_hq2x <= 0;
                pxl_rep_c_x_hq2x <= 0;
                next_reset_line <= 0;
                counterX_reg_vga <= 12'b_1111_11111111;

                if (`StartInputCounter(counterX_reg_q, counterY_reg_q)) begin
                    ram_addrY_reg_hq2x <= 0;
                    next_reset_frame <= 0;
                    counterY_reg_vga <= 0;
                end else begin
                    if (ram_addrY_reg_hq2x < hdmiVideoConfig.ram_numwords - hdmiVideoConfig.buffer_line_length) begin
                        ram_addrY_reg_hq2x <= ram_addrY_reg_hq2x + hdmiVideoConfig.buffer_line_length;
                    end else begin
                        ram_addrY_reg_hq2x <= 0;
                    end
                    counterY_reg_vga <= counterY_reg_vga + 1'b1;
                end
            end else begin
                if (pxl_rep_c_x_hq2x == PXL_REP_H_HQ2X - 1) begin
                    if (ram_addrX_reg_hq2x < hdmiVideoConfig.buffer_line_length - 1) begin
                        ram_addrX_reg_hq2x <= ram_addrX_reg_hq2x + 1'b1;
                    end else begin
                        ram_addrX_reg_hq2x <= 0;
                        next_reset_line <= 1;
                        if (`StopInputCounter(counterX_reg_q, counterY_reg_q)) begin
                            next_reset_frame <= 1;
                        end
                    end
                    pxl_rep_c_x_hq2x <= 0;
                    counterX_reg_vga <= counterX_reg_vga + 1'b1;
                end else begin
                    pxl_rep_c_x_hq2x <= pxl_rep_c_x_hq2x + 1'b1;
                end
            end
            d_rdaddr <= `GetAddr_f(counterX_reg_q, counterY_reg_q);

            //////////////////////////////////////////////////////////////////////
            // generate base counter
            if (counterX_reg < hdmiVideoConfig.horizontal_pixels_per_line - 1) begin
                counterX_reg <= counterX_reg + 1'b1;
            end else begin
                counterX_reg <= 0;

                if (counterY_reg < `VerticalLines_f - 1) begin
                    counterY_reg <= counterY_reg + 1'b1;
                end else begin
                    counterY_reg <= 0;
                    state <= ~state;
                end
            end
            if (counterY_reg == SKIP_LINES) begin
                counterY_shift_q <= 0;
            end else if (counterY_reg > SKIP_LINES) begin
                counterY_shift_q <= counterY_reg - SKIP_LINES;
            end else begin
                counterY_shift_q <= 12'h_f00;
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
            if (counterY_shift_q >= `VerticalSyncStart_f 
             && counterY_shift_q < `VerticalSyncStart_f + hdmiVideoConfig.vertical_sync_width + 1) 
            begin
                if ((counterY_shift_q == `VerticalSyncStart_f 
                    && counterX_reg_q < `VerticalSyncPixelOffset_f) 
                 || (counterY_shift_q == `VerticalSyncStart_f + hdmiVideoConfig.vertical_sync_width 
                    && counterX_reg_q >= `VerticalSyncPixelOffset_f)) 
                begin
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
            // OSD TEXT
            //////////////////////////////////////////////////////////////////////
            // generate ram read
            if (`AdvanceOSDInputCounter(counterX_reg_q, counterY_reg_q)) begin
                text_rdaddr_x <= 0;
                pxl_rep_c_x_osd <= 0;
                pxl_rep_c_x_osd_pxl <= 0;
                counterX_osd_reg <= 0;

                if (`StartOSDInputCounter(counterX_reg_q, counterY_reg_q)) begin
                    text_rdaddr_y <= 0;
                    counterY_osd_reg <= 0;
                    charPixelRow_reg <= 0;
                end else begin
                    if (charPixelRow_reg == OSD_FONT_HEIGHT - 1) begin
                        text_rdaddr_y <= text_rdaddr_y + 10'd40;
                        counterY_osd_reg <= counterY_osd_reg + 1'b1;
                    end
                    charPixelRow_reg <= charPixelRow_reg + 1'b1;
                end
            end else begin
                if (pxl_rep_c_x_osd == OSD_PXL_REP_H_HQ2X - 1) begin
                    if (text_rdaddr_x < 40) begin
                        text_rdaddr_x <= text_rdaddr_x + 1'b1;
                    end
                    pxl_rep_c_x_osd <= 0;
                end else begin
                    pxl_rep_c_x_osd <= pxl_rep_c_x_osd + 1'b1;
                end
                
                if (pxl_rep_c_x_osd_pxl == PXL_REP_H_HQ2X - 1) begin
                    counterX_osd_reg <= counterX_osd_reg + 1'b1;
                    pxl_rep_c_x_osd_pxl <= 0;
                end else begin
                    pxl_rep_c_x_osd_pxl <= pxl_rep_c_x_osd_pxl + 1'b1;
                end
            end

            //////////////////////////////////////////////////////////////////////
            // SCANLINES
            if (scanline.active) begin
                if (scanline.dopre) begin
                    isScanline <= counterY_shift_q_q_q_q[2:1] >> scanline.thickness ^ scanline.oddeven;
                end else begin
                    isScanline <= counterY_shift_q_q_q_q[1:0] >> scanline.thickness ^ scanline.oddeven;
                end
            end else begin
                isScanline <= 1'b0;
            end

            //////////////////////////////////////////////////////////////////////
            // delay queue
            counterX_reg_q <= counterX_reg;
            counterX_reg_q_q <= counterX_reg_q;
            counterX_reg_q_q_q <= counterX_reg_q_q;
            counterX_reg_q_q_q_q <= counterX_reg_q_q_q;

            counterY_reg_q <= counterY_reg;

            counterY_shift_q_q <= counterY_shift_q;
            counterY_shift_q_q_q <= counterY_shift_q_q;
            counterY_shift_q_q_q_q <= counterY_shift_q_q_q;

            hsync_reg_q_q <= hsync_reg_q;
            vsync_reg_q_q <= vsync_reg_q;
            hsync_reg_q_q_q <= hsync_reg_q_q;
            vsync_reg_q_q_q <= vsync_reg_q_q;

            //////////////////////////////////////////////////////////////////////
            // OUTPUT
            fullcycle <= fullcycle || _fullcycle == 4'b1111;

            _d_video_out <= `GetData_f(counterX_reg_out, counterY_shift_out);
            _d_DrawArea <= `IsDrawAreaHDMI_f(counterX_reg_out, counterY_shift_out);
            _d_hsync <= hsync_reg_out;
            _d_vsync <= vsync_reg_out;

            d_video_out <= _d_video_out;
            d_DrawArea <= _d_DrawArea;
            d_hsync <= _d_hsync;
            d_vsync <= _d_vsync;
        end
    end

    delayline #(
        .CYCLES(4),
        .WIDTH(26)
    ) output_delay (
        .clock(clock),
        .in({ counterX_reg_q_q_q_q, counterY_shift_q_q_q_q, hsync_reg_q_q_q, vsync_reg_q_q_q }),
        .out({ counterX_reg_out, counterY_shift_out, hsync_reg_out, vsync_reg_out })
    );

    assign text_rdaddr = text_rdaddr_x + text_rdaddr_y[9:0];
    assign char_addr = (text_rddata << 4) + charPixelRow_reg;
    assign rdaddr = d_rdaddr;
    assign video_out = d_video_out;
    assign DrawArea = d_DrawArea;
    assign hsync = d_hsync;
    assign vsync = d_vsync;

endmodule