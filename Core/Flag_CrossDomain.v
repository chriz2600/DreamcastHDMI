// http://www.fpga4fun.com/CrossClockDomain2.html

module Flag_CrossDomain(
    input clkA,
    input FlagIn_clkA,
    //output Busy_clkA,
    input clkB,
    output FlagOut_clkB
);
    reg FlagToggle_clkA;
    reg [2:0] SyncA_clkB;

    always @(posedge clkA) begin
        FlagToggle_clkA <= FlagToggle_clkA ^ FlagIn_clkA;  // when flag is asserted, this signal toggles (clkA domain)
    end

    always @(posedge clkB) begin
        SyncA_clkB <= {SyncA_clkB[1:0], FlagToggle_clkA};  // now we cross the clock domains
    end

    assign FlagOut_clkB = (SyncA_clkB[2] ^ SyncA_clkB[1]);  // and create the clkB flag

// reg data_out_meta;
// reg[3:0] rCount;
// reg[1:0] data_out_reg;

// assign FlagOut_clkB = data_out_reg[1];

// always @(posedge clkA) begin
//     if (FlagIn_clkA) begin
//         rCount <= 4'b1000;
//     end else if (rCount > 0) begin
//         rCount <= rCount - 1'b1;
//     end
// end

// always @(rCount) begin
//     data_out_meta <= (rCount > 0);
// end

// always @(posedge clkB) begin
//     data_out_reg <= { data_out_reg[0], data_out_meta };
// end
endmodule
