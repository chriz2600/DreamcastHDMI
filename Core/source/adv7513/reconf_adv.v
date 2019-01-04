module reconf_adv (
    input clock,
    input rdempty,
    input [7:0] fdata,
    output reg rdreq,
    output ADV7513Config adv7513Config
);

`include "config/adv_config.v"

initial begin
    adv7513Config <= ADV7513_CONFIG_1080P;
end

always @(posedge clock) begin

    if (~rdempty) begin
        rdreq <= 1'b1;
    end else begin
        rdreq <= 1'b0;
    end

    if (rdreq) begin
        case (fdata[6:0])
            7'h00: adv7513Config <= ADV7513_CONFIG_1080P;
            7'h01: adv7513Config <= ADV7513_CONFIG_960P;
            7'h02: adv7513Config <= ADV7513_CONFIG_480P;
            7'h03: adv7513Config <= ADV7513_CONFIG_VGA;

            7'h10: adv7513Config <= ADV7513_CONFIG_240P_1080P;
            7'h11: adv7513Config <= ADV7513_CONFIG_240P_960P;
            7'h12: adv7513Config <= ADV7513_CONFIG_240P_480P;
            7'h13: adv7513Config <= ADV7513_CONFIG_240P_VGA;

            7'h20: adv7513Config <= ADV7513_CONFIG_480I;
            7'h21: adv7513Config <= ADV7513_CONFIG_480I;
            7'h22: adv7513Config <= ADV7513_CONFIG_480I;
            7'h23: adv7513Config <= ADV7513_CONFIG_480I;

            7'h40: adv7513Config <= ADV7513_CONFIG_576I;
            7'h41: adv7513Config <= ADV7513_CONFIG_576I;
            7'h42: adv7513Config <= ADV7513_CONFIG_576I;
            7'h43: adv7513Config <= ADV7513_CONFIG_576I;
        endcase
    end
end

endmodule
