module data_cross(
    input clkIn,
    input clkOut,
    input [7:0] dataIn,
    output reg [7:0] dataOut
);
    reg wrreq = 0;
    reg rdreq = 0;
    reg wrfull;
    reg rdempty;
    reg [7:0] dataIn_reg = 0;

    reconf_fifo	fifo(
        .wrclk(clkIn),
        .data(dataIn_reg),
        .wrreq(wrreq),
        .wrfull(wrfull),

        .rdclk(clkOut),
        .rdreq(rdreq),
        .rdempty(rdempty),
        .q(dataOut)
    );

    always @(posedge clkIn) begin
        dataIn_reg <= dataIn;
        if (dataIn_reg != dataIn) begin
            wrreq <= 1'b1;
        end else begin
            wrreq <= 1'b0;
        end
    end

    always @(posedge clkOut) begin
        if (~rdempty) begin
            rdreq <= 1'b1;
        end else begin
            rdreq <= 1'b0;
        end
    end

endmodule
