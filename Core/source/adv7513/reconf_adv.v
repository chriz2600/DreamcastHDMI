module reconf_adv (
    input clock,
    input [7:0] data_in,
    output ADV7513Config adv7513Config,
    output adv7513_reconf
);

    `include "config/adv_config.v"

    reg [7:0] data_in_reg = 0;
    reg adv7513_reconf_reg;

    ADV7513Config adv7513Config_reg;
    initial begin
        adv7513Config_reg <= ADV7513_CONFIG_1080P;
    end
    assign adv7513Config = adv7513Config_reg;
    assign adv7513_reconf = adv7513_reconf_reg;

    always @(posedge clock) begin

        data_in_reg <= data_in;

        if (data_in_reg != data_in) begin
            adv7513_reconf_reg <= 1'b1;
            case (data_in[6:0])
                7'h00: adv7513Config_reg <= ADV7513_CONFIG_1080P;
                7'h01: adv7513Config_reg <= ADV7513_CONFIG_960P;
                7'h02: adv7513Config_reg <= ADV7513_CONFIG_480P;
                7'h03: adv7513Config_reg <= ADV7513_CONFIG_VGA;

                7'h10: adv7513Config_reg <= ADV7513_CONFIG_240P_1080P;
                7'h11: adv7513Config_reg <= ADV7513_CONFIG_240P_960P;
                7'h12: adv7513Config_reg <= ADV7513_CONFIG_240P_480P;
                7'h13: adv7513Config_reg <= ADV7513_CONFIG_240P_VGA;

                7'h20: adv7513Config_reg <= ADV7513_CONFIG_480I;
                7'h21: adv7513Config_reg <= ADV7513_CONFIG_480I;
                7'h22: adv7513Config_reg <= ADV7513_CONFIG_480I;
                7'h23: adv7513Config_reg <= ADV7513_CONFIG_480I;

                7'h40: adv7513Config_reg <= ADV7513_CONFIG_576I;
                7'h41: adv7513Config_reg <= ADV7513_CONFIG_576I;
                7'h42: adv7513Config_reg <= ADV7513_CONFIG_576I;
                7'h43: adv7513Config_reg <= ADV7513_CONFIG_576I;
            endcase
        end else begin
            adv7513_reconf_reg <= 0;
        end
    end
endmodule
