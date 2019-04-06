/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */

module ram2video_f(
    input clock,
    input reset,
    input starttrigger,
    input hq2x,
    output reg fullcycle,
    
    output [13:0] rdaddr,
    input [23:0] rddata,

    //input line_doubler,
    input HDMIVideoConfig hdmiVideoConfig,

    // video output
    output [23:0] video_out,
    output hsync,
    output vsync,
    output DrawArea
);

    // localparam HORIZONTAL_PIXELS_PER_LINE = 2250;
    // localparam HORIZONTAL_PIXELS_VISIBLE = 1920;
    // localparam VERTICAL_LINES_VISIBLE = 1080;
    // localparam HORIZONTAL_CAPTURE_START = 320;
    // localparam HORIZONTAL_CAPTURE_END = 1600;
    // localparam VERTICAL_CAPTURE_START = 60;
    // localparam VERTICAL_CAPTURE_END = 1020;
    // localparam VERTICAL_LINES_1 = 1100;
    // localparam VERTICAL_LINES_2 = 1100;
    // localparam VERTICAL_SYNC_START_1 = 1083;
    // localparam VERTICAL_SYNC_START_2 = 1083;
    // localparam VERTICAL_SYNC_PIXEL_OFFSET_1 = 2008;
    // localparam VERTICAL_SYNC_PIXEL_OFFSET_2 = 2008;
    // localparam HORIZONTAL_SYNC_ON_POLARITY = 1;
    // localparam VERTICAL_SYNC_ON_POLARITY = 1;
    // localparam RAM_NUMWORDS = 14720;
    // localparam BUFFER_LINE_LENGTH = 640;
    // localparam HORIZONTAL_SYNC_START = 2008;
    // localparam HORIZONTAL_SYNC_WIDTH = 44;
    // localparam vertical_sync_width = 5;
    localparam PXL_REP_H_HQ2X = 4;
    localparam SKIP_FRAMES = 12'd3;
    localparam DATA_DELAY_START = 4;
    localparam DATA_DELAY_END = 2;

    wire [23:0] outpixel;
    Hq2x_optimized hq2x_inst (
        .clk(clock),
        .ce_x4(1'b1),
        .inputpixel(reset_frame_q_q | reset_line_q_q ? 24'h_00 : { rddata[7:0], rddata[15:8], rddata[23:16] }),
        .mono(1'b0),
        .disable_hq2x(~hq2x),
        .reset_frame(reset_frame_q_q),
        .reset_line(reset_line_q_q),
        .read_y(read_y),
        .hblank(hblank),
        .outpixel(outpixel)
    );

    /* verilator lint_off UNSIGNED */

    `define IsHBlank(x, y) (x >= hdmiVideoConfig.horizontal_capture_end + DATA_DELAY_END && x < hdmiVideoConfig.horizontal_pixels_per_line - DATA_DELAY_START)
    `define ResetReadY(y) (y == `VerticalLines - 1)
    `define AdvanceReadY(x, y) (x == hdmiVideoConfig.horizontal_capture_end + DATA_DELAY_END && (y >= SKIP_FRAMES + hdmiVideoConfig.vertical_capture_start))

    `define IsDrawAreaHDMI_f(x, y)   (x >= 0 && x < hdmiVideoConfig.horizontal_pixels_visible \
                                 && `AdjustedCounterY(y) >= 0 && `AdjustedCounterY(y) < hdmiVideoConfig.vertical_lines_visible)

    `define IsDrawAreaVGA(x, y)   (x >= hdmiVideoConfig.horizontal_capture_start \
                                && x < hdmiVideoConfig.horizontal_capture_end \
                                && y >= hdmiVideoConfig.vertical_capture_start \
                                && y < hdmiVideoConfig.vertical_capture_end)

    `define GetAddr_f(x, y) (next_reset_frame | next_reset_line ? 14'b0 : ram_addrY_reg_hq2x + { 4'b0, ram_addrX_reg_hq2x })
    `define GetData_f(x, y) (`IsDrawAreaHDMI_f(x, y) ? { outpixel[7:0], outpixel[15:8], outpixel[23:16] } : 24'h00)

    reg [9:0] ram_addrX_reg_hq2x;
    reg [13:0] ram_addrY_reg_hq2x;

    `define VerticalLines (state ? hdmiVideoConfig.vertical_lines_2 : hdmiVideoConfig.vertical_lines_1)
    `define VerticalSyncStart (state ? hdmiVideoConfig.vertical_sync_start_2 : hdmiVideoConfig.vertical_sync_start_1)
    `define VerticalSyncPixelOffset (state ? hdmiVideoConfig.vertical_sync_pixel_offset_2 : hdmiVideoConfig.vertical_sync_pixel_offset_1)

    `define AdjustedCounterY(y) (y >= SKIP_FRAMES ? y - SKIP_FRAMES : 12'h_f00)

    `define StartInputCounter(x, y) (x == hdmiVideoConfig.horizontal_capture_start && y == hdmiVideoConfig.vertical_capture_start && y[0] == 0)
    `define StopInputCounter(x, y) (y == hdmiVideoConfig.vertical_capture_end - 1)
    `define AdvanceInputCounter(x, y) (x == hdmiVideoConfig.horizontal_capture_start && y >= hdmiVideoConfig.vertical_capture_start && y[0] == 0)

    reg trigger /*verilator public*/;
    reg state /*verilator public*/;
    reg [11:0] counterX_reg /*verilator public*/;
    reg [11:0] counterX_reg_q /*verilator public*/;
    reg [11:0] counterX_reg_q_q /*verilator public*/;
    reg [11:0] counterX_reg_q_q_q /*verilator public*/;
    reg [11:0] counterY_reg /*verilator public*/;
    reg [11:0] counterY_reg_q /*verilator public*/;
    reg [11:0] counterY_reg_q_q /*verilator public*/;
    reg [11:0] counterY_reg_q_q_q /*verilator public*/;

    reg hsync_reg_q /*verilator public*/;
    reg vsync_reg_q /*verilator public*/;
    reg hsync_reg_q_q /*verilator public*/;
    reg vsync_reg_q_q /*verilator public*/;

    /* verilator lint_off UNUSED */
    reg [13:0] d_rdaddr /*verilator public*/;

    reg [23:0] _d_video_out;
    reg _d_hsync;
    reg _d_vsync;
    reg _d_DrawArea;

    reg [23:0] d_video_out;
    reg d_hsync;
    reg d_vsync;
    reg d_DrawArea;

    reg [3:0] pxl_rep_c_x_hq2x;

    reg next_reset_line /*verilator public*/;
    reg reset_line /*verilator public*/;
    reg reset_line_q /*verilator public*/;
    reg reset_line_q_q /*verilator public*/;
    reg next_reset_frame /*verilator public*/;
    reg reset_frame /*verilator public*/;
    reg reset_frame_q /*verilator public*/;
    reg reset_frame_q_q /*verilator public*/;
    reg hblank /*verilator public*/;
    reg [1:0] read_y /*verilator public*/;
    reg [3:0] _fullcycle;

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
                hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
                vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity;
                ram_addrX_reg_hq2x <= 0;
                ram_addrY_reg_hq2x <= 0;
                next_reset_line <= 1;
                reset_line <= 1;
                reset_line_q <= 1;
                reset_line_q_q <= 1;
                pxl_rep_c_x_hq2x <= 0;
                state <= 0;
                read_y <= 2'd2;
            end
        end else begin
            //////////////////////////////////////////////////////////////////////
            // trigger is set, output data
            //////////////////////////////////////////////////////////////////////

            //////////////////////////////////////////////////////////////////////
            // generate read_y and hblank signals
            if (`ResetReadY(counterY_reg)) begin
                read_y <= 2'd2;
            end else if (`AdvanceReadY(counterX_reg, counterY_reg)) begin
                read_y <= read_y + 1'b1;
            end
            hblank <= `IsHBlank(counterX_reg, counterY_reg);

            //////////////////////////////////////////////////////////////////////
            // generate ram read
            if (`AdvanceInputCounter(counterX_reg, counterY_reg)) begin
                ram_addrX_reg_hq2x <= 0;
                pxl_rep_c_x_hq2x <= 0;
                next_reset_line <= 0;

                if (`StartInputCounter(counterX_reg, counterY_reg)) begin
                    ram_addrY_reg_hq2x <= 0;
                    next_reset_frame <= 0;
                end else begin
                    if (ram_addrY_reg_hq2x < hdmiVideoConfig.ram_numwords - hdmiVideoConfig.buffer_line_length) begin
                        ram_addrY_reg_hq2x <= ram_addrY_reg_hq2x + hdmiVideoConfig.buffer_line_length;
                    end else begin
                        ram_addrY_reg_hq2x <= 0;
                    end
                end
            end else begin
                if (pxl_rep_c_x_hq2x == PXL_REP_H_HQ2X - 1) begin
                    if (ram_addrX_reg_hq2x < hdmiVideoConfig.buffer_line_length - 1) begin
                        ram_addrX_reg_hq2x <= ram_addrX_reg_hq2x + 1'b1;
                    end else begin
                        ram_addrX_reg_hq2x <= 0;
                        next_reset_line <= 1;
                        if (`StopInputCounter(counterX_reg, counterY_reg)) begin
                            next_reset_frame <= 1;
                        end
                    end
                    pxl_rep_c_x_hq2x <= 0;
                end else begin
                    pxl_rep_c_x_hq2x <= pxl_rep_c_x_hq2x + 1'b1;
                end
            end
            reset_line <= next_reset_line;
            reset_line_q <= reset_line;
            reset_line_q_q <= reset_line_q;
            reset_frame <= next_reset_frame;
            reset_frame_q <= reset_frame;
            reset_frame_q_q <= reset_frame_q;

            //////////////////////////////////////////////////////////////////////
            // generate base counter
            if (counterX_reg < hdmiVideoConfig.horizontal_pixels_per_line - 1) begin
                counterX_reg <= counterX_reg + 1'b1;
            end else begin
                counterX_reg <= 0;

                if (counterY_reg < `VerticalLines - 1) begin
                    counterY_reg <= counterY_reg + 1'b1;
                end else begin
                    counterY_reg <= 0;
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
            if (`AdjustedCounterY(counterY_reg_q) >= `VerticalSyncStart 
             && `AdjustedCounterY(counterY_reg_q) < `VerticalSyncStart + hdmiVideoConfig.vertical_sync_width + 1) 
            begin
                if ((`AdjustedCounterY(counterY_reg_q) == `VerticalSyncStart 
                    && counterX_reg_q < `VerticalSyncPixelOffset) 
                 || (`AdjustedCounterY(counterY_reg_q) == `VerticalSyncStart + hdmiVideoConfig.vertical_sync_width 
                    && counterX_reg_q >= `VerticalSyncPixelOffset)) 
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
            // OUTPUT
            d_rdaddr <= `GetAddr_f(counterX_reg, counterY_reg);
            if (fullcycle || _fullcycle >= 4'b0001) begin
                _d_video_out <= `GetData_f(counterX_reg_q_q_q, counterY_reg_q_q_q);
                _d_DrawArea <= `IsDrawAreaHDMI_f(counterX_reg_q_q_q, counterY_reg_q_q_q);
                _d_hsync <= hsync_reg_q_q;
                _d_vsync <= vsync_reg_q_q;
                if (_fullcycle == 4'b1111) begin
                    fullcycle <= 1;
                end
            end else begin
                _d_video_out <= 24'd0;
                _d_DrawArea <= 1'b0;
                _d_hsync <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
                _d_vsync <= ~hdmiVideoConfig.vertical_sync_on_polarity;
            end
        end
    end

    delayline #(
        .CYCLES(2),
        .WIDTH(27)
    ) vout_delay (
        .clock(clock),
        .in({ _d_video_out, _d_DrawArea, _d_hsync, _d_vsync }),
        .out({ d_video_out, d_DrawArea, d_hsync, d_vsync })
    );

    assign rdaddr = d_rdaddr;
    assign video_out = d_video_out;
    assign DrawArea = d_DrawArea;
    assign hsync = d_hsync;
    assign vsync = d_vsync;

endmodule