module trigger_reconf(
    input clock,
    input wrfull,
    input [7:0] data_in,
    output reg [7:0] data,
    output reg wrreq
);

reg [7:0] data_in_reg = 0;

always @(posedge clock) begin
    data_in_reg <= data_in;

    if (data_in_reg != data_in) begin
        wrreq <= 1'b1;
        data <= data_in;
    end else begin
        wrreq <= 1'b0;
    end
end

endmodule