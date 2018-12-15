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
        case (fdata[3:0])
            0: begin
                adv7513Config <= ADV7513_CONFIG_1080P;
            end
            1: begin
                adv7513Config <= ADV7513_CONFIG_960P;
            end
            2: begin
                adv7513Config <= ADV7513_CONFIG_480P;
            end
            3: begin
                adv7513Config <= ADV7513_CONFIG_VGA;
            end
            4: begin
                adv7513Config <= ADV7513_CONFIG_240Px3;
            end
        endcase
    end
end

endmodule
