// http://www.fpga4fun.com/CrossClockDomain2.html

module Flag_CrossDomain(
    input clkA,
    input FlagIn_clkA,
    //output Busy_clkA,
    input clkB,
    output FlagOut_clkB
);

reg FlagToggle_clkA;
always @(posedge clkA) FlagToggle_clkA <= FlagToggle_clkA ^ (FlagIn_clkA & ~(FlagToggle_clkA ^ SyncB_clkA[1]));

reg [2:0] SyncA_clkB;
always @(posedge clkB) SyncA_clkB <= {SyncA_clkB[1:0], FlagToggle_clkA};

reg [1:0] SyncB_clkA;
always @(posedge clkA) SyncB_clkA <= {SyncB_clkA[0], SyncA_clkB[2]};

assign FlagOut_clkB = (SyncA_clkB[2] ^ SyncA_clkB[1]);

endmodule
