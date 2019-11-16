`include "config.inc"

module gamma(
    input clock,
    input [4:0] gamma_config,
    input [7:0] in,
    output reg [7:0] out
);

    always @(posedge clock) begin
        case (gamma_config)
            `GAMMA_0_714290: begin
                case (in)
                    `include "config/gamma_0_714290.v"
                endcase
            end
            `GAMMA_0_769231: begin
                case (in)
                    `include "config/gamma_0_769231.v"
                endcase
            end
            `GAMMA_0_833330: begin
                case (in)
                    `include "config/gamma_0_833330.v"
                endcase
            end
            `GAMMA_0_909090: begin
                case (in)
                    `include "config/gamma_0_909090.v"
                endcase
            end
            `GAMMA_1_1: begin
                case (in)
                    `include "config/gamma_1_1.v"
                endcase
            end
            `GAMMA_1_2: begin
                case (in)
                    `include "config/gamma_1_2.v"
                endcase
            end
            `GAMMA_1_3: begin
                case (in)
                    `include "config/gamma_1_3.v"
                endcase
            end
            `GAMMA_1_4: begin
                case (in)
                    `include "config/gamma_1_4.v"
                endcase
            end
            default: out <= in;
        endcase
    end

endmodule