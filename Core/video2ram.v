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
    output [14:0] wraddr,
    output wren,
    output wrclock,
    
    output starttrigger
);

    reg [9:0] H_CAPTURE_START;
    reg [9:0] H_CAPTURE_END;
    reg [9:0] H_TRIGGER_POINT;
    reg [9:0] V_CAPTURE_START;
    reg [9:0] V_CAPTURE_END;
    reg [9:0] V_TRIGGER_POINT;

    reg wren_reg = 0;
    reg [23:0] wrdata_reg;
    reg [14:0] wraddr_reg;
    reg [11:0] tmp;
    reg trigger = 0;

    always @(*) begin
        if (line_doubler) begin
            H_CAPTURE_START = 10'd1;
            H_CAPTURE_END   = 10'd641;
            H_TRIGGER_POINT = 10'd320;
            V_CAPTURE_START = 10'd0;
            V_CAPTURE_END   = 10'd504;
            V_TRIGGER_POINT = 10'd1;
        end else begin
            H_CAPTURE_START = 10'd44;
            H_CAPTURE_END   = 10'd684;
            H_TRIGGER_POINT = 10'd320;
            V_CAPTURE_START = 10'd0;
            V_CAPTURE_END   = 10'd480;
            V_TRIGGER_POINT = 10'd0;
        end
    end
    
    `define GetWriteAddr(x, y) (((x - H_CAPTURE_START) * ((y - V_CAPTURE_START) % `BUFFER_SIZE)) + (x - H_CAPTURE_START))
    `define IsFirstBuffer(y)   ((y - V_CAPTURE_START) <= `BUFFER_SIZE)


    always @ (posedge clock) begin
    
        if (line_doubler) begin
            // 480i/240p mode
            if (counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
                wraddr_reg[9:0] <= counterX[9:0] - H_CAPTURE_START;
                
                if (counterY < 240) begin
                    wren_reg <= 1;
                    wraddr_reg[11:10] <= counterY[1:0];
                    wrdata_reg <= { R, G, B };
                end else if (counterY > 262 && counterY < 504) begin
                    tmp = counterY - 12'd3;
                    wren_reg <= 1;
                    wraddr_reg[11:10] <= tmp[1:0];
                    wrdata_reg <= { R, G, B };
                end else begin
                    wren_reg <= 0;
                end
                
                if (counterX == H_TRIGGER_POINT && counterY == V_TRIGGER_POINT) begin
                    trigger <= 1'b1;
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
                
                if (`IsFirstBuffer(counterY) && `GetWriteAddr(counterX, counterY) == `TRIGGER_ADDR) begin
                    trigger <= 1'b1;
                end else begin
                    trigger <= 1'b0;
                end
            end else begin
                wren_reg <= 0;
                wraddr_reg <= 15'd0;
                wrdata_reg <= 24'd0;
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