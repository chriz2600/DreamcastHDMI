`include "config.inc"

module gammaconv(
    input clock,
    
    input [4:0] gamma_config,

    input in_wren,
    input [`RAM_WIDTH-1:0] in_wraddr,
    input [7:0] in_red,
    input [7:0] in_green,
    input [7:0] in_blue,
    input in_starttrigger,

    output reg wren,
    output reg [`RAM_WIDTH-1:0] wraddr,
    output reg [23:0] wrdata,
    output reg starttrigger
);

    wire [7:0] red;
    wire [7:0] green;
    wire [7:0] blue;

    gamma r(
        .clock(clock),
        .gamma_config(gamma_config),
        .in(in_red),
        .out(red)
    );

    gamma g(
        .clock(clock),
        .gamma_config(gamma_config),
        .in(in_green),
        .out(green)
    );

    gamma b(
        .clock(clock),
        .gamma_config(gamma_config),
        .in(in_blue),
        .out(blue)
    );

    reg wren_q;
    reg [`RAM_WIDTH-1:0] wraddr_q;
    reg starttrigger_q;

    always @(posedge clock) begin
        { wren, wren_q } <= { wren_q, in_wren };
        { wraddr, wraddr_q } <= { wraddr_q, in_wraddr };
        { starttrigger, starttrigger_q } <= { starttrigger_q, in_starttrigger };
        wrdata <= { red, green, blue };
    end

endmodule