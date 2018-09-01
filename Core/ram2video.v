`include "config.inc"

module ram2video(
    input [23:0] rddata,
    input starttrigger,

    input clock,
    input reset,
    
    input line_doubler,
    input add_line,

    output [14:0] rdaddr,
    input [7:0] text_rddata,
    output [9:0] text_rdaddr,
    output [23:0] video_out,
    
    output hsync,
    output vsync,
    
    output DrawArea,
    output reg restart,

    input enable_osd,
    input [7:0] highlight_line,
    input HDMIVideoConfig hdmiVideoConfig,
    input Scanline scanline,
    output reg fullcycle
);
    reg [10:0] vlines; // vertical lines per frame
    
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
    
    reg [11:0] counterY_reg;
    reg [11:0] counterY_reg_q;
    reg [11:0] counterY_reg_q_q;
    reg [11:0] counterY_reg_q_q_q;
    
    reg [7:0] currentLine_reg;
    reg [7:0] currentLine_reg_q;

    reg [3:0] charPixelRow_reg;

    reg [9:0] ram_addrX_reg;
    reg [14:0] ram_addrY_reg;

    reg trigger = 1'b0;
    //reg fullcycle = 1'b0;

    wire ld_rise;
    wire ld_fall;
    wire al_rise;
    wire al_fall;

    wire combined_reset;
    assign combined_reset = ~reset || ld_rise || ld_fall || al_rise || al_fall;

    edge_detect lineDoubler(
        .async_sig(line_doubler),
        .clk(clock),
        .rise(ld_rise),
        .fall(ld_fall)
    );

    edge_detect addLine(
        .async_sig(add_line),
        .clk(clock),
        .rise(al_rise),
        .fall(al_fall)
    );

    wire [10:0] char_addr;
    wire [7:0] char_data;
    char_rom char_rom_inst(
        .address(char_addr),
        .clock(clock),
        .q(char_data)
    );

    initial begin
        trigger <= 0;
        counterX_reg <= `INITIAL_HORIZONTAL_OFFSET;
        counterY_reg <= `INITIAL_VERTICAL_OFFSET;
        hsync_reg_q <= ~`INITIAL_HORIZONTAL_SYNC_ON_POLARITY;
        vsync_reg_q <= ~`INITIAL_VERTICAL_SYNC_ON_POLARITY;
        ram_addrX_reg <= 0;
        ram_addrY_reg <= 0;
    end
    
    task doReset;
        input triggered;
    
        begin
            trigger <= triggered;
            counterX_reg <= hdmiVideoConfig.horizontal_offset;
            counterY_reg <= hdmiVideoConfig.vertical_offset;
            hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
            vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity;
            ram_addrX_reg <= 0;
            ram_addrY_reg <= 0;
        end
    endtask	

    always @(*) begin
        if (add_line) begin
            vlines <= hdmiVideoConfig.vertical_lines_240p;
        end else begin
            vlines <= hdmiVideoConfig.vertical_lines;
        end
    end
    
    always @(posedge clock) begin
        if (combined_reset) begin
            doReset(1'b0);
            restart <= 0;
            fullcycle <= 0;
        end else if (!trigger) begin
            // wait for trigger to start
            if (starttrigger) begin
                restart <= 1;
                doReset(1'b1);
            end
        end else begin
            restart <= 0;
            if (counterX_reg_q == 0 && counterY_reg_q == 0) begin
                fullcycle <= 1;
            end
            // trigger is set, output data
            if (counterX_reg < hdmiVideoConfig.horizontal_pixels_per_line - 1) begin
                counterX_reg <= counterX_reg + 1'b1;

                if (counterX_reg >= hdmiVideoConfig.horizontal_offset
                 && ram_addrX_reg < hdmiVideoConfig.buffer_line_length - 1) begin
                    ram_addrX_reg <= ram_addrX_reg + 1'b1;
                end else begin
                    ram_addrX_reg <= 0;
                end
            end else begin
                counterX_reg <= 0;
                ram_addrX_reg <= 0;

                if (counterY_reg < vlines - 1) begin
                    counterY_reg <= counterY_reg + 1'b1;

                    if (counterY_reg >= hdmiVideoConfig.vertical_offset
                     && ram_addrY_reg < hdmiVideoConfig.ram_numwords - hdmiVideoConfig.buffer_line_length) begin
                        if (hdmiVideoConfig.pixel_repetition) begin
                            if (counterY_reg[0] && (!line_doubler || counterY_reg[1])) begin
                                ram_addrY_reg <= ram_addrY_reg + hdmiVideoConfig.buffer_line_length;
                            end
                        end else begin
                            if (!line_doubler || counterY_reg[0]) begin
                                ram_addrY_reg <= ram_addrY_reg + hdmiVideoConfig.buffer_line_length;
                            end
                        end
                        //$display("2: y:%0d ay:%0d", counterY_reg, ram_addrY_reg);
                    end else begin
                        if (hdmiVideoConfig.pixel_repetition) begin
                            if (counterY_reg[0]) begin
                                ram_addrY_reg <= 0;
                            end
                        end else begin
                            ram_addrY_reg <= 0;
                        end
                        //$display("2: y:%0d ay:%0d", counterY_reg, ram_addrY_reg);
                    end
                end else begin
                    counterY_reg <= 0;
                    ram_addrY_reg <= 0;
                end
            end

            // generate output hsync
            if (counterX_reg_q >= hdmiVideoConfig.horizontal_sync_start && counterX_reg_q < hdmiVideoConfig.horizontal_sync_start + hdmiVideoConfig.horizontal_sync_width) begin
                hsync_reg_q <= hdmiVideoConfig.horizontal_sync_on_polarity;
            end else begin
                hsync_reg_q <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
            end

            // generate output vsync
            if (counterY_reg_q >= hdmiVideoConfig.vertical_sync_start && counterY_reg_q < hdmiVideoConfig.vertical_sync_start + hdmiVideoConfig.vertical_sync_width + 1) begin // + 1: synchronize last vsync period with hsync negative edge
                if ((counterY_reg_q == hdmiVideoConfig.vertical_sync_start && counterX_reg_q < hdmiVideoConfig.horizontal_sync_start) 
                    || (counterY_reg_q == hdmiVideoConfig.vertical_sync_start + hdmiVideoConfig.vertical_sync_width && counterX_reg_q >= hdmiVideoConfig.horizontal_sync_start)) begin
                    vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity; // OFF
                end else begin
                    vsync_reg_q <= hdmiVideoConfig.vertical_sync_on_polarity; // ON
                end
            end else begin
                vsync_reg_q <= ~hdmiVideoConfig.vertical_sync_on_polarity; // OFF
            end

            counterX_reg_q <= counterX_reg;
            counterY_reg_q <= counterY_reg;
            counterX_reg_q_q <= counterX_reg_q;
            counterY_reg_q_q <= counterY_reg_q;
            counterX_reg_q_q_q <= counterX_reg_q_q;
            counterY_reg_q_q_q <= counterY_reg_q_q;
            hsync_reg_q_q <= hsync_reg_q;
            vsync_reg_q_q <= vsync_reg_q;
        end
    end

    localparam ONE_TO_ONE = 256;

    `ifdef OSD_BACKGROUND_ALPHA
        localparam OSD_BACKGROUND_ALPHA = `OSD_BACKGROUND_ALPHA;
    `else
        localparam OSD_BACKGROUND_ALPHA = 64;
    `endif

    reg [7:0] char_data_req;
    reg [31:0] text_rddata_reg;
    reg [9:0] text_rdaddr_x;
    reg [9:0] text_rdaddr_y;
    always @(posedge clock) begin
        if (counterX_reg == 0) begin
            if (hdmiVideoConfig.pixel_repetition) begin
                currentLine_reg <= counterY_reg[11:5] - hdmiVideoConfig.text_offset_character_y[7:0];
                charPixelRow_reg <= counterY_reg[4:1];
            end else begin
                currentLine_reg <= counterY_reg[11:4] - hdmiVideoConfig.text_offset_character_y[7:0];
                charPixelRow_reg <= counterY_reg[3:0];
            end
        end else if (counterX_reg == 1) begin
            text_rdaddr_y <= currentLine_reg * 10'd40;
            text_rdaddr_x <= 0;
        end else if (counterX_reg[11:3] >= hdmiVideoConfig.text_offset_character_x) begin
            text_rdaddr_x <= (counterX_reg[11:3] - hdmiVideoConfig.text_offset_character_x[9:0] + 1'b1);
        end
        text_rddata_reg[7:0] <= text_rddata;
        text_rddata_reg[15:8] <= text_rddata_reg[7:0];
        text_rddata_reg[23:16] <= text_rddata_reg[15:8];
        text_rddata_reg[31:24] <= text_rddata_reg[23:16];
        char_data_req <= char_data;
        currentLine_reg_q <= currentLine_reg;
    end

    assign text_rdaddr = text_rdaddr_x + text_rdaddr_y;
    // bit 7 of text_rddata_reg[31:24] is currently ignored, as we only have a 7 bit charset, maybe this could be used as invert character indicator later
    assign char_addr = (text_rddata_reg[31:24] << 4) + charPixelRow_reg;

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

    `define GetData(x, y) ( \
        `IsDrawAreaVGA(x, y) \
        ?   `IsOsdTextArea(x, y) \
            ?   (char_data_req[7-counterX_reg_q_q[2:0]]) ^ (currentLine_reg_q == highlight_line) \
                ?   {24{1'b1}} \
                :   `GetRdData(y, (isScanline ? truncate_osdbg(OSD_BACKGROUND_ALPHA * scanline.intensity) : OSD_BACKGROUND_ALPHA)) \
            :   `IsOsdBgArea(x, y) \
                ?   `GetRdData(y, (isScanline ? truncate_osdbg(OSD_BACKGROUND_ALPHA * scanline.intensity) : OSD_BACKGROUND_ALPHA)) \
                :   `GetRdData(y, (isScanline ? scanline.intensity : ONE_TO_ONE)) \
        :   24'h00 \
        )

    `define IsDrawAreaHDMI(x, y)   (x >= 0 && x < hdmiVideoConfig.horizontal_pixels_visible \
                                 && y >= 0 && y < hdmiVideoConfig.vertical_lines_visible)

    `define IsDrawAreaVGA(x, y)   (x >= hdmiVideoConfig.horizontal_capture_start \
                                && x < hdmiVideoConfig.horizontal_capture_end \
                                && y >= hdmiVideoConfig.vertical_capture_start \
                                && y < hdmiVideoConfig.vertical_capture_end)

    reg isScanline = 0;
    always @(posedge clock) begin
        if (scanline.active) begin
            if (hdmiVideoConfig.pixel_repetition) begin
                isScanline <= counterY_reg[2:1] >> scanline.thickness ^ scanline.oddeven;
            end else begin
                isScanline <= counterY_reg[1:0] >> scanline.thickness ^ scanline.oddeven;
            end
        end else begin
            isScanline <= 1'b0;
        end
    end

    `define GetAddr(x, y) (`IsDrawAreaVGA(x, y) ? ram_addrY_reg + ram_addrX_reg : 15'd0)

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

    `define GetRdData(y, a) ({ \
                truncate_rddata({ 8'b0, rddata[23:16] } * a), \
                truncate_rddata({ 8'b0, rddata[15:8] } * a), \
                truncate_rddata({ 8'b0, rddata[7:0] } * a) \
            })

    reg [14:0] d_rdaddr;
    reg [23:0] d_video_out;
    reg d_hsync;
    reg d_vsync;
    reg d_DrawArea;
    
    always @(posedge clock) begin
        d_rdaddr <= `GetAddr(counterX_reg, counterY_reg);
        if (fullcycle) begin
            d_video_out <= `GetData(counterX_reg_q_q_q, counterY_reg_q_q_q);
            d_DrawArea <= `IsDrawAreaHDMI(counterX_reg_q_q_q, counterY_reg_q_q_q);
            d_hsync <= hsync_reg_q_q;
            d_vsync <= vsync_reg_q_q;
        end else begin
            d_video_out <= 24'd0;
            d_DrawArea <= 1'b0;
            d_hsync <= ~hdmiVideoConfig.horizontal_sync_on_polarity;
            d_vsync <= ~hdmiVideoConfig.vertical_sync_on_polarity;
        end
    end

    assign rdaddr = d_rdaddr;
    assign video_out = d_video_out;
    assign DrawArea = d_DrawArea;
    assign hsync = d_hsync;
    assign vsync = d_vsync;

    // assign rdaddr = `GetAddr(counterX_reg, counterY_reg);
    // assign video_out = fullcycle ? `GetData(counterX_reg_q_q, counterY_reg_q_q) : 24'd0;
    // assign hsync = fullcycle ? hsync_reg_q : ~hdmiVideoConfig.horizontal_sync_on_polarity;
    // assign vsync = fullcycle ? vsync_reg_q : ~hdmiVideoConfig.vertical_sync_on_polarity;
    // assign DrawArea = fullcycle ? `IsDrawAreaHDMI(counterX_reg_q_q, counterY_reg_q_q) : 1'b0;

endmodule