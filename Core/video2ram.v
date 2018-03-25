`include "config.inc"

module video2ram(
    input clock,
    
    input [7:0] R,
    input [7:0] G,
    input [7:0] B,
    input [11:0] counterX,
    input [11:0] counterY,
    
    input line_doubler,
    
    output [23:0] wrdata,
    output [`RAM_ADDRESS_BITS-1:0] wraddr,
    output wren,
    output wrclock,
    
    output starttrigger
);

    reg [9:0] H_CAPTURE_START;
    reg [9:0] H_CAPTURE_END;
    reg [9:0] V_CAPTURE_START;
    reg [9:0] V_CAPTURE_END;

    reg wren_reg;
    reg [23:0] wrdata_reg;
    reg [`RAM_ADDRESS_BITS-1:0] wraddr_reg;
    reg [`RAM_ADDRESS_BITS-1:0] ram_addrY_reg;
    reg trigger;

    always @(*) begin
        if (line_doubler) begin
            H_CAPTURE_START = 10'd1;
            H_CAPTURE_END   = 10'd641;
            V_CAPTURE_START = 10'd0;
            V_CAPTURE_END   = 10'd504;
        end else begin
            H_CAPTURE_START = 10'd44;
            H_CAPTURE_END   = 10'd684;
            V_CAPTURE_START = 10'd0;
            V_CAPTURE_END   = 10'd480;
        end
    end
    
    //`define GetWriteAddr(x, y) ((`BUFFER_LINE_LENGTH * ((y - V_CAPTURE_START) % `BUFFER_SIZE)) + (x - H_CAPTURE_START))
    `define GetWriteAddr(x) (ram_addrY_reg + (x - H_CAPTURE_START))
    `define IsFirstBuffer(y)   ((y - V_CAPTURE_START) < `BUFFER_SIZE)
    `define IsTriggerPoint(y) (`IsFirstBuffer(y) && wraddr_reg == `TRIGGER_ADDR)

    `define IsVerticalCaptureTime(y) ( \
        line_doubler \
            ? (y < 240 || (y > 262 && y < V_CAPTURE_END)) \
            : (y >= V_CAPTURE_START && y < V_CAPTURE_END) \
    )
    `define IsCaptureTime(x,y) ( \
        `IsVerticalCaptureTime(y) && x >= H_CAPTURE_START && x < H_CAPTURE_END \
    )

    initial begin
        wren_reg <= 0;
        wrdata_reg <= 24'd0;
        wraddr_reg <= 0;
        ram_addrY_reg <= 0;
        trigger <= 0;
    end

    always @ (posedge clock) begin

        if (counterX == H_CAPTURE_END) begin // calculate ram_addrY_reg once per line
            if (`IsVerticalCaptureTime(counterY)
             && ram_addrY_reg < `RAM_NUMWORDS - `BUFFER_LINE_LENGTH) begin
                ram_addrY_reg <= ram_addrY_reg + `BUFFER_LINE_LENGTH;
            end else begin
                ram_addrY_reg <= 0;
            end
        end

        if (`IsCaptureTime(counterX, counterY)) begin
            wren_reg <= 1;
            wraddr_reg <= `GetWriteAddr(counterX);
            wrdata_reg <= { R, G, B };

            if (`IsTriggerPoint(counterY)) begin
                trigger <= 1'b1;
            end else begin
                trigger <= 1'b0;
            end
        end else begin
            wren_reg <= 0;
            trigger <= 1'b0;
        end
    end

    assign wren = wren_reg;
    assign wrclock = clock;
    assign wraddr = wraddr_reg;
    assign wrdata = wrdata_reg;
    assign starttrigger = trigger;
    
endmodule