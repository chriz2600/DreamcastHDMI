module edge_detect(
    input async_sig,
    input clk,
    output reg rise,
    output reg fall
);

reg [1:3] resync;

always @(posedge clk) begin
    rise <= resync[2] && !resync[3];
    fall <= resync[3] && !resync[2];
    resync <= { async_sig, resync[1:2] };
end

endmodule