module trigger_reconf(
    input clock,
    input wrfull,
    input [7:0] data_in,
    output reg [7:0] data,
    output reg wrreq,
    output HDMIVideoConfig hdmiVideoConfig
);

`include "../config/hdmi_config.v"

reg [7:0] data_in_reg = 0;

HDMIVideoConfig hdmiVideoConfig_reg;
initial begin
    hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
end
assign hdmiVideoConfig = hdmiVideoConfig_reg;

always @(posedge clock) begin
    data_in_reg <= data_in;

    if (data_in_reg != data_in) begin
        wrreq <= 1'b1;
        data <= data_in;
        //wrreq_reg <= 1'b1;
        case (data_in[6:0])
            // 480p input
            7'h00: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
            7'h01: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_960P;
            7'h02: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480P;
            7'h03: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_VGA;

            7'h10: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_1080P;
            7'h11: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_960P;
            7'h12: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_480P;
            7'h13: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_VGA;

            7'h20: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
            7'h21: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
            7'h22: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
            7'h23: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;

            7'h40: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
            7'h41: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
            7'h42: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
            7'h43: hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
        endcase
    end else begin
        wrreq <= 1'b0;
    end
end

endmodule