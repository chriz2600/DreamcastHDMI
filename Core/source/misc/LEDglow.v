// https://www.fpga4fun.com/Opto2.html

module LEDglow(
    input clk,
    output LED
);
    parameter BITPOS = 23;

    reg [BITPOS:0] cnt;

    always @(posedge clk) cnt <= cnt + 1'b1;
    wire [3:0] PWM_input = cnt[BITPOS] ? cnt[BITPOS-1:BITPOS-4] : ~cnt[BITPOS-1:BITPOS-4];    // ramp the PWM input up and down

    reg [4:0] PWM;
    always @(posedge clk) PWM <= PWM[3:0]+PWM_input;

    assign LED = PWM[4];
endmodule