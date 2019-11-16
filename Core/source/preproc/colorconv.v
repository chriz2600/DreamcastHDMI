`include "config.inc"

module colorconv(
    input clock,
    
    input [7:0] color_config_data,

    input in_wren,
    input [`RAM_WIDTH-1:0] in_wraddr,
    input [23:0] in_wrdata,
    input in_starttrigger,

    output reg wren,
    output reg [`RAM_WIDTH-1:0] wraddr,
    output reg [23:0] wrdata,
    output reg starttrigger
);

    always @(posedge clock) begin
        wren <= in_wren;
        wraddr <= in_wraddr;
        case (color_config_data)
            `RGB555: wrdata <= {
                in_wrdata[23:16] | in_wrdata[23:16] >> 5,
                in_wrdata[15:8] | in_wrdata[15:8] >> 5,
                in_wrdata[7:0] | in_wrdata[7:0] >> 5,
            }; // RGB555
            `RGB565: wrdata <= {
                in_wrdata[23:16] | in_wrdata[23:16] >> 5,
                in_wrdata[15:8] | in_wrdata[15:8] >> 6,
                in_wrdata[7:0] | in_wrdata[7:0] >> 5,
            }; // RGB565
            default: wrdata <= in_wrdata; // RGB888
        endcase
        starttrigger <= in_starttrigger;
    end

endmodule