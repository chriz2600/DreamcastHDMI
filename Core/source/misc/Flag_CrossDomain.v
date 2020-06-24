// http://www.fpga4fun.com/CrossClockDomain2.html

module Flag_CrossDomain(
    input clkA,
    input FlagIn_clkA,
    //output Busy_clkA,
    input clkB,
    output FlagOut_clkB
);
    parameter INIT_STATE = 0;

    reg data_out = INIT_STATE;
    reg [3:0] rCount;
    reg [2:0] data_out_reg;

    assign FlagOut_clkB = data_out_reg[2];

    always @(posedge clkA) begin
        if (FlagIn_clkA) begin
            rCount <= 4'b1000;
            data_out <= 1'b1;
        end else if (rCount > 0) begin
            rCount <= rCount - 1'b1;
        end else begin
            data_out <= 1'b0;
        end
    end

    always @(posedge clkB) begin
        data_out_reg <= { data_out_reg[1], data_out_reg[0], data_out };
    end
endmodule
