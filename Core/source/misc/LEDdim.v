module LEDdim(
    input clk,
    output LED
);
    parameter BITPOS = 2;

    reg [BITPOS:0] cnt;
    reg _led;

    always @(posedge clk) begin
        cnt <= cnt + 1'b1;
        if (cnt == 0) begin
            _led <= 1'b1;
        end else begin
            _led <= 1'b0;
        end
    end

    assign LED = _led;
endmodule