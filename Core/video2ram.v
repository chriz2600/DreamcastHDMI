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

    reg wren_reg = 0;
    reg [23:0] wrdata_reg;
    reg [`RAM_ADDRESS_BITS-1:0] wraddr_reg;
    reg [11:0] tmp;
    reg trigger = 0;

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
    
    `define GetWriteAddr(x, y) (((x - H_CAPTURE_START) * ((y - V_CAPTURE_START) % `BUFFER_SIZE)) + (x - H_CAPTURE_START))
    `define IsFirstBuffer(y)   ((y - V_CAPTURE_START) <= `BUFFER_SIZE)
    `define IsTriggerPoint(x, y) (`IsFirstBuffer(y) && `GetWriteAddr(x, y) == `TRIGGER_ADDR)

    always @ (posedge clock) begin
    
        if (line_doubler) begin
            // 480i/240p mode
            if (counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
                wraddr_reg <= `GetWriteAddr(counterX, counterY);
                
                if (counterY < 240 || (counterY > 262 && counterY < 504)) begin
                    wren_reg <= 1;
                    wrdata_reg <= { R, G, B };
                end else begin
                    wren_reg <= 0;
                end
                
                if (`IsTriggerPoint(counterX, counterY)) begin
                    trigger <= 1'b1;
                end else begin
                    trigger <= 1'b0;
                end
            end else begin
                wren_reg <= 0;
                trigger <= 1'b0;
            end

        end else begin
            // 480p mode
            if (counterY >= V_CAPTURE_START && counterY < V_CAPTURE_END && counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
                wren_reg <= 1;
                wraddr_reg <= `GetWriteAddr(counterX, counterY);
                wrdata_reg <= { R, G, B };
                
                if (`IsTriggerPoint(counterX, counterY)) begin
                    trigger <= 1'b1;
                end else begin
                    trigger <= 1'b0;
                end
            end else begin
                wren_reg <= 0;
                trigger <= 1'b0;
            end
        end
    end

    assign wren = wren_reg;
    assign wrclock = clock;
    assign wraddr = wraddr_reg;
    assign wrdata = wrdata_reg;
    assign starttrigger = trigger;
    
    
endmodule