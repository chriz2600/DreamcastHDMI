// https://www.fpga4fun.com/CrossClockDomain1.html

module Signal_CrossDomain(
    input SignalIn_clkA,
    input clkB,
    output SignalOut_clkB
);

// We use a four-stages shift-register to synchronize SignalIn_clkA to the clkB clock domain
reg [3:0] SyncA_clkB;
always @(posedge clkB) begin 
    SyncA_clkB[0] <= SignalIn_clkA;
    SyncA_clkB[1] <= SyncA_clkB[0];
    SyncA_clkB[2] <= SyncA_clkB[1];
    SyncA_clkB[3] <= SyncA_clkB[2];
end

assign SignalOut_clkB = SyncA_clkB[3];  // new signal synchronized to (=ready to be used in) clkB domain
endmodule