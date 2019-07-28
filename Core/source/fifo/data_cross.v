
module data_cross(
    input clkIn,
    input clkOut,
    input [WIDTH-1:0] dataIn,
    output reg [WIDTH-1:0] dataOut
);
    parameter WIDTH = 8;
    
    reg wrreq = 0 /* synthesis noprune */;
    reg rdreq = 0 /* synthesis noprune */;
    reg wrfull /* synthesis noprune */;
    reg rdempty /* synthesis noprune */;
    reg [WIDTH-1:0] dataIn_reg = 0 /* synthesis noprune */;

    data_fifo #(
        .WIDTH(WIDTH)
    ) fifo (
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
