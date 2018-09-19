module I2C(
    // clock and reset
    input clk,
    input reset,

    // inputs
    input [6:0] chip_addr,
    input [7:0] reg_addr,
    input [7:0] value,
    input enable,
    input is_read,

    // I2C pins
    inout sda,
    inout scl,

    // outputs
    output reg [7:0] data,
    output reg done,
    output i2c_ack_error,
    input [31:0] divider
);

reg i2c_ena;
reg [6:0] i2c_addr;
reg i2c_rw;
reg [7:0] i2c_data_wr;
reg i2c_busy;
reg [7:0] i2c_data_rd;

i2c_master i2c_master(
    .clk       (clk),
    .reset_n   (1'b1),
    .ena       (i2c_ena),
    .addr      (i2c_addr),
    .rw        (i2c_rw),
    .data_wr   (i2c_data_wr),
    .busy      (i2c_busy),
    .data_rd   (i2c_data_rd),
    .ack_error (i2c_ack_error),
    .sda       (sda),
    .scl       (scl),
    .divider   (divider)
);

(* syn_encoding = "safe" *)
reg [1:0] state;
reg [5:0] busy_cnt;
reg busy_prev;

reg [6:0] chip_addr_reg;
reg [7:0] reg_addr_reg;
reg [7:0] value_reg;

localparam  s_idle = 0,
            s_send = 1;

initial begin
    state <= s_idle;
    busy_cnt = 0;
    done <= 1'b1;
end

always @ (posedge clk) begin
    if (~reset) begin
        state <= s_idle;
        busy_cnt = 0;
        done <= 1'b1;
    end
    else begin
        case(state)
            s_idle: begin
                if (enable) begin
                    chip_addr_reg <= chip_addr;
                    reg_addr_reg <= reg_addr;
                    value_reg <= value;
                    done <= 1'b0;
                    state <= s_send;
                end
            end
            
            s_send: begin
                busy_prev <= i2c_busy;
                if (~busy_prev && i2c_busy) begin
                    busy_cnt = busy_cnt + 1'b1;
                end
                case (busy_cnt)
                    0: begin
                        i2c_ena <= 1'b1;
                        i2c_addr <= chip_addr_reg;
                        i2c_rw <= 1'b0;
                        i2c_data_wr <= reg_addr_reg;
                    end
                    
                    1: begin
                        if (is_read) begin
                            i2c_rw <= 1'b1;
                        end else begin
                            i2c_data_wr <= value_reg;
                        end
                    end
                    
                    2: begin
                        i2c_ena <= 1'b0;
                        if (~i2c_busy) begin
                            data <= i2c_data_rd;
                            busy_cnt = 0;
                            done <= 1'b1;
                            state <= s_idle;
                        end
                    end
                endcase
            end
        endcase
    end
end

endmodule

