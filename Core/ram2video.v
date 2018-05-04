`include "config.inc"

module ram2video(
    input [23:0] rddata,
    input starttrigger,

    input clock,
    input reset,
    
    input line_doubler,
    input add_line,

    output [`RAM_ADDRESS_BITS-1:0] rdaddr,
`ifdef DEBUG
    input [7:0] text_rddata,
    output [9:0] text_rdaddr,
`endif    
    output [23:0] video_out,
    
    output hsync,
    output vsync,
    
    output DrawArea,
    output videoClock,
    output reg restart
);

    reg [10:0] vlines; // vertical lines per frame
    
    reg [7:0] red_reg;
    reg [7:0] green_reg;
    reg [7:0] blue_reg;

    reg hsync_reg_q = 1'b1;
    reg vsync_reg_q = 1'b1;

    reg [11:0] counterX_reg;
    reg [11:0] counterX_reg_q;
    reg [11:0] counterX_reg_q_q;
    
    reg [11:0] counterY_reg;
    reg [11:0] counterY_reg_q;
    reg [11:0] counterY_reg_q_q;
    
    reg [9:0] ram_addrX_reg;
    reg [`RAM_ADDRESS_BITS-1:0] ram_addrY_reg;

    reg trigger = 1'b0;

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

`ifdef DEBUG
    wire [10:0] char_addr;
    wire [7:0] char_data;
    char_rom char_rom_inst(
        .address(char_addr),
        .clock(clock),
        .q(char_data)
    );
`endif

    initial begin
        doReset(1'b0);
    end
    
    task doReset;
        input triggered;
    
        begin
            trigger <= triggered;
            counterX_reg <= `HORIZONTAL_OFFSET;
            counterY_reg <= `VERTICAL_OFFSET;
            hsync_reg_q <= ~`HORIZONTAL_SYNC_ON_POLARITY;
            vsync_reg_q <= ~`VERTICAL_SYNC_ON_POLARITY;
            ram_addrX_reg <= 0;
            ram_addrY_reg <= 0;
        end
    endtask	

    always @(*) begin
        if (add_line) begin
            vlines <= `VERTICAL_LINES_240P;
        end else begin
            vlines <= `VERTICAL_LINES;
        end
    end
    
    always @(posedge clock) begin
        if (combined_reset) begin
            doReset(1'b0);
            restart <= 0;
        end else if (!trigger) begin
            // wait for trigger to start
            if (starttrigger) begin
                restart <= 1;
                doReset(1'b1);
            end
        end else begin
            restart <= 0;
            // trigger is set, output data
            if (counterX_reg < `HORIZONTAL_PIXELS_PER_LINE - 1) begin
                counterX_reg <= counterX_reg + 1'b1;

                if (counterX_reg >= `HORIZONTAL_OFFSET
                 && ram_addrX_reg < `BUFFER_LINE_LENGTH - 1) begin
                    ram_addrX_reg <= ram_addrX_reg + 1'b1;
                end else begin
                    ram_addrX_reg <= 0;
                end
            end else begin
                counterX_reg <= 0;
                ram_addrX_reg <= 0;

                if (counterY_reg < vlines - 1) begin
                    counterY_reg <= counterY_reg + 1'b1;

                    if (counterY_reg >= `VERTICAL_OFFSET
                     && ram_addrY_reg < `RAM_NUMWORDS - `BUFFER_LINE_LENGTH) begin
`ifdef PIXEL_REPETITION
                        if (counterY_reg[0]) begin
                            ram_addrY_reg <= ram_addrY_reg + `BUFFER_LINE_LENGTH;
                        end
`else
                        ram_addrY_reg <= ram_addrY_reg + `BUFFER_LINE_LENGTH;
`endif
                        //$display("2: y:%0d ay:%0d", counterY_reg, ram_addrY_reg);

                    end else begin
`ifdef PIXEL_REPETITION
                        if (counterY_reg[0]) begin
                            ram_addrY_reg <= 0;
                        end
`else
                        ram_addrY_reg <= 0;
`endif
                        //$display("2: y:%0d ay:%0d", counterY_reg, ram_addrY_reg);
                    end
                end else begin
                    counterY_reg <= 0;
                    ram_addrY_reg <= 0;
                end
            end

            // generate output hsync
            if (counterX_reg_q >= `HORIZONTAL_SYNC_START && counterX_reg_q < `HORIZONTAL_SYNC_START + `HORIZONTAL_SYNC_WIDTH) begin
                hsync_reg_q <= `HORIZONTAL_SYNC_ON_POLARITY;
            end else begin
                hsync_reg_q <= ~`HORIZONTAL_SYNC_ON_POLARITY;
            end

            // generate output vsync
            if (counterY_reg_q >= `VERTICAL_SYNC_START && counterY_reg_q < `VERTICAL_SYNC_START + `VERTICAL_SYNC_WIDTH + 1) begin // + 1: synchronize last vsync period with hsync negative edge
                if ((counterY_reg_q == `VERTICAL_SYNC_START && counterX_reg_q < `HORIZONTAL_SYNC_START) 
                    || (counterY_reg_q == `VERTICAL_SYNC_START + `VERTICAL_SYNC_WIDTH && counterX_reg_q >= `HORIZONTAL_SYNC_START)) begin
                    vsync_reg_q <= ~`VERTICAL_SYNC_ON_POLARITY; // OFF
                end else begin
                    vsync_reg_q <= `VERTICAL_SYNC_ON_POLARITY; // ON
                end
            end else begin
                vsync_reg_q <= ~`VERTICAL_SYNC_ON_POLARITY; // OFF
            end

            counterX_reg_q <= counterX_reg;
            counterY_reg_q <= counterY_reg;
            counterX_reg_q_q <= counterX_reg_q;
            counterY_reg_q_q <= counterY_reg_q;
        end
    end

`ifdef OSD_BACKGROUND_ALPHA
    localparam OSD_BACKGROUND_ALPHA = `OSD_BACKGROUND_ALPHA;
`else
    localparam OSD_BACKGROUND_ALPHA = 16;
`endif
`ifdef SCANLINES_INTENSITY
    localparam SCANLINES_INTENSITY = `SCANLINES_INTENSITY;
`else
    localparam SCANLINES_INTENSITY = 16;
`endif
`ifdef DEBUG
    localparam TEXT_OFFSET_CHARACTER_X = (`TEXT_OFFSET_COUNTER_X + `HORIZONTAL_OFFSET) / `TEXT_WIDTH_DIVIDER;
    localparam TEXT_OFFSET_CHARACTER_Y = (`TEXT_OFFSET_COUNTER_Y + `VERTICAL_OFFSET) / `TEXT_HEIGHT_DIVIDER;

    reg [7:0] char_data_req;
    reg [31:0] text_rddata_reg;
    reg [9:0] text_rdaddr_x;
    reg [11:0] text_rdaddr_y;
    always @(posedge clock) begin
        if (counterX_reg == 0) begin
            text_rdaddr_y <= (counterY_reg[11:`TEXT_RD_ADDR_LOWER_BIT_Y] - TEXT_OFFSET_CHARACTER_Y) * 40;
            text_rdaddr_x <= 0;
        end else if (counterX_reg[11:`TEXT_RD_ADDR_LOWER_BIT_X] >= TEXT_OFFSET_CHARACTER_X) begin
            text_rdaddr_x <= (counterX_reg[11:`TEXT_RD_ADDR_LOWER_BIT_X] - TEXT_OFFSET_CHARACTER_X + 1'b1);
        end
        text_rddata_reg[7:0] <= text_rddata;
        text_rddata_reg[15:8] <= text_rddata_reg[7:0];
        text_rddata_reg[23:16] <= text_rddata_reg[15:8];
        text_rddata_reg[31:24] <= text_rddata_reg[23:16];
        char_data_req <= char_data;
    end

    assign text_rdaddr = text_rdaddr_x + text_rdaddr_y;
    assign char_addr = (text_rddata_reg[31:24] << 4) + counterY_reg[`TEXT_RD_ADDR_LOWER_BIT_Y-1:`TEXT_CHAR_ADDR_LOWER_BIT_Y];

    `define IsDrawAreaText(x, y, paddingX, paddingY)  ( \
        x >= `HORIZONTAL_OFFSET + `TEXT_OFFSET_COUNTER_X - paddingX \
        && x < `HORIZONTAL_PIXELS_VISIBLE - `HORIZONTAL_OFFSET - `TEXT_OFFSET_COUNTER_X + paddingX \
        && y >= `VERTICAL_OFFSET + `TEXT_OFFSET_COUNTER_Y - paddingY \
        && y < `VERTICAL_LINES_VISIBLE - `VERTICAL_OFFSET - `TEXT_OFFSET_COUNTER_Y + paddingY)

    `define GetData(x, y) (`IsDrawAreaVGA(x, y) ? \
        (`IsDrawAreaText(x, y, `TEXT_PADDING_X, `TEXT_PADDING_Y) ? \
            (`IsDrawAreaText(x, y, `TEXT_PADDING_X2, `TEXT_PADDING_Y2) && char_data_req[7-counterX_reg_q_q[2:0]] ? {24{1'b1}} \
                : `GetRdData(y, OSD_BACKGROUND_ALPHA)) \
            : `GetRdData(y, 16)) \
        : 24'h00)
`else 
    `define GetData(x, y) (`IsDrawAreaVGA(x, y) ? (`IsScanline(y) ? `GetRdData(y, SCANLINES_INTENSITY) : rddata) : 24'h00)
`endif

    `define IsDrawAreaHDMI(x, y)   (x >= 0 && x < `HORIZONTAL_PIXELS_VISIBLE \
                                 && y >= 0 && y < `VERTICAL_LINES_VISIBLE)

    `define IsDrawAreaVGA(x, y)   (x >= `HORIZONTAL_OFFSET \
                                && x < `HORIZONTAL_PIXELS_VISIBLE - `HORIZONTAL_OFFSET \
                                && y >= `VERTICAL_OFFSET \
                                && y < `VERTICAL_LINES_VISIBLE - `VERTICAL_OFFSET)
    `ifdef SCANLINES_EVEN
        `ifdef SCANLINES_THICK
            `define IsScanline(y) (y[1])
        `else
            `define IsScanline(y) (y[0])
        `endif
    `elsif SCANLINES_ODD
        `ifdef SCANLINES_THICK
            `define IsScanline(y) (~y[1])
        `else
            `define IsScanline(y) (~y[0])
        `endif
    `else
        `define IsScanline(y) (0)
    `endif
    `define GetAddr(x, y) (`IsDrawAreaVGA(x, y) ? ram_addrY_reg + ram_addrX_reg : `RAM_ADDRESS_BITS'd0)

    function [7:0] truncate_rddata(
        input[11:0] value
    );
        truncate_rddata = value[7:0];
    endfunction

    `define GetRdData(y, a) ({ \
                truncate_rddata(((rddata[23:16] << 4) / a)), \
                truncate_rddata(((rddata[15:8] << 4) / a)), \
                truncate_rddata(((rddata[7:0] << 4) / a)) \
            })

    assign rdaddr = `GetAddr(counterX_reg, counterY_reg);
    assign video_out = `GetData(counterX_reg_q_q, counterY_reg_q_q);
    assign hsync = hsync_reg_q;
    assign vsync = vsync_reg_q;
    assign DrawArea = `IsDrawAreaHDMI(counterX_reg_q_q, counterY_reg_q_q);
    assign videoClock = clock ^ `INVERT_VIDEO_CLOCK; 

endmodule