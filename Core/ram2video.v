`include "config.inc"

module ram2video(
    input [23:0] rddata,
    input starttrigger,

    input clock,
    input reset,
    
    input line_doubler,
    input add_line,

    output [`RAM_ADDRESS_BITS-1:0] rdaddr,
    
    output [7:0] red,
    output [7:0] green,
    output [7:0] blue,
    
    output hsync,
    output vsync,
    
    output DrawArea,
    output videoClock
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
        end else if (!trigger) begin
            // wait for trigger to start
            if (starttrigger) begin
                doReset(1'b1);
            end
        end else begin
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

    `define IsDrawAreaHDMI(x, y)   (x >= 0 && x < `HORIZONTAL_PIXELS_VISIBLE \
                                 && y >= 0 && y < `VERTICAL_LINES_VISIBLE)

    `define IsDrawAreaVGA(x, y)   (x >= `HORIZONTAL_OFFSET \
                                && x < `HORIZONTAL_PIXELS_VISIBLE - `HORIZONTAL_OFFSET \
                                && y >= `VERTICAL_OFFSET \
                                && y < `VERTICAL_LINES_VISIBLE - `VERTICAL_OFFSET)

    `define GetAddr(x, y) (`IsDrawAreaVGA(x, y) ? ram_addrY_reg + ram_addrX_reg : `RAM_ADDRESS_BITS'd0)
    //`define GetAddr(x, y) (ram_addrY_reg + ram_addrX_reg)
    `define GetData(t,b) (`IsDrawAreaVGA(counterX_reg_q_q, counterY_reg_q_q) ? rddata[t:b] : 8'h00)

    assign rdaddr = `GetAddr(counterX_reg, counterY_reg);
    assign red = `GetData(23, 16);
    assign green = `GetData(15, 8);
    assign blue = `GetData(7, 0);
    assign hsync = hsync_reg_q;
    assign vsync = vsync_reg_q;
    assign DrawArea = `IsDrawAreaHDMI(counterX_reg_q_q, counterY_reg_q_q);
    assign videoClock = clock ^ `INVERT_VIDEO_CLOCK; 

endmodule