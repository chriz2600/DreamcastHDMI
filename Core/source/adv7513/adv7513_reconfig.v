module adv7513_reconfig (
    input clock,
    input [7:0] data_in,
    input [7:0] clock_config_data,
    input [1:0] colorspace_in,
    output ADV7513Config adv7513Config,
    output [7:0] clock_data_out,
    output [1:0] colorspace_out,
    output adv7513_reconf
);

    `include "../config/adv_config.v"

    reg [7:0] data_in_reg = 0;
    reg [1:0] colorspace_in_reg = 0;
    reg [7:0] clock_config_data_reg = 0;
    reg [7:0] clock_data_out_reg = 8'b_0110_0000;
    reg [1:0] colorspace_reg = 0;
    reg adv7513_reconf_reg;

    ADV7513Config adv7513Config_reg;
    initial begin
        adv7513Config_reg <= ADV7513_CONFIG_1080P;
    end
    assign adv7513Config = adv7513Config_reg;
    assign clock_data_out = clock_data_out_reg;
    assign adv7513_reconf = adv7513_reconf_reg;
    assign colorspace_out = colorspace_reg;

    always @(posedge clock) begin

        data_in_reg <= data_in;
        colorspace_in_reg <= colorspace_in;
        clock_config_data_reg <= clock_config_data;

        if (data_in_reg != data_in
         || colorspace_in_reg != colorspace_in
         || clock_config_data_reg != clock_config_data
        ) begin
            adv7513_reconf_reg <= 1'b1;
            if (data_in_reg != data_in) begin
                case (data_in[6:0])
                    // RECONF
                    7'h00: adv7513Config_reg <= ADV7513_CONFIG_1080P;
                    7'h01: adv7513Config_reg <= ADV7513_CONFIG_960P;
                    7'h02: adv7513Config_reg <= ADV7513_CONFIG_480P;
                    7'h03: adv7513Config_reg <= ADV7513_CONFIG_VGA;

                    7'h04: adv7513Config_reg <= ADV7513_CONFIG_288P;
                    7'h05: adv7513Config_reg <= ADV7513_CONFIG_288P;
                    7'h06: adv7513Config_reg <= ADV7513_CONFIG_288P;
                    7'h07: adv7513Config_reg <= ADV7513_CONFIG_288P;

                    7'h08: adv7513Config_reg <= ADV7513_CONFIG_576P;
                    7'h09: adv7513Config_reg <= ADV7513_CONFIG_576P;
                    7'h0A: adv7513Config_reg <= ADV7513_CONFIG_576P;
                    7'h0B: adv7513Config_reg <= ADV7513_CONFIG_576P;

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
            end
            if (clock_config_data_reg != clock_config_data) begin
                //clock_data_out_reg <= clock_config_data[7:5] << 5;
                case (clock_config_data)
                    0: clock_data_out_reg <= 8'b_0000_0000; // -1.2ns
                    1: clock_data_out_reg <= 8'b_0010_0000; // -0.8ns
                    2: clock_data_out_reg <= 8'b_0100_0000; // -0.4ns
                    3: clock_data_out_reg <= 8'b_0110_0000; // no delay
                    4: clock_data_out_reg <= 8'b_1000_0000; // 0.4ns
                    5: clock_data_out_reg <= 8'b_1010_0000; // 0.8ns
                    6: clock_data_out_reg <= 8'b_1100_0000; // 1.2ns
                    7: clock_data_out_reg <= 8'b_1110_0000; // 1.6ns
                endcase
            end
            colorspace_reg <= colorspace_in;
        end else begin
            adv7513_reconf_reg <= 0;
        end
    end
endmodule
