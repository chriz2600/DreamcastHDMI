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
            4'h0: begin
                adv7513Config <= ADV7513_CONFIG_1080P;
            end
            4'h1: begin
                adv7513Config <= ADV7513_CONFIG_960P;
            end
            4'h2: begin
                adv7513Config <= ADV7513_CONFIG_480P;
            end
            4'h3: begin
                adv7513Config <= ADV7513_CONFIG_VGA;
            end
            4'h8: begin
                adv7513Config <= ADV7513_CONFIG_240P_1080P;
            end
            4'h9: begin
                adv7513Config <= ADV7513_CONFIG_240P_960P;
            end
            4'hA: begin
                adv7513Config <= ADV7513_CONFIG_240P_480P;
            end
            4'hB: begin
                adv7513Config <= ADV7513_CONFIG_240P_VGA;
            end
        endcase
    end
end

endmodule
