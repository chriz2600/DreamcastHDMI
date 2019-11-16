`include "config.inc"

module colorconv(
    input clock,
    
    input [2:0] color_config,

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

    always @(posedge clock) begin
        wren <= in_wren;
        wraddr <= in_wraddr;
        case (color_config)
            `RGB555: wrdata <= {
                in_red | in_red >> 5,
                in_green | in_green >> 5,
                in_blue | in_blue >> 5,
            }; // RGB555
            `RGB565: wrdata <= {
                in_red | in_red >> 5,
                in_green | in_green >> 6,
                in_blue | in_blue >> 5,
            }; // RGB565
            default: wrdata <= { in_red, in_green, in_blue }; // RGB888
        endcase
        starttrigger <= in_starttrigger;
    end

endmodule