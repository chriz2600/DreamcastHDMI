module hdmi_video_reconfig(
    input clock,
    input [7:0] data_in,
    output HDMIVideoConfig hdmiVideoConfig
);

    `include "config/hdmi_config.v"

    reg [7:0] data_in_reg = 0;

    HDMIVideoConfig _hdmiVideoConfig_reg;
    HDMIVideoConfig hdmiVideoConfig_reg;
    initial begin
        _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
        hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
    end
    assign hdmiVideoConfig = hdmiVideoConfig_reg;

    always @(posedge clock) begin
        data_in_reg <= data_in;

        if (data_in_reg != data_in) begin
            case (data_in[6:0])
                // RECONF
                7'h00: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
                7'h01: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_960P;
                7'h02: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480P;
                7'h03: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_VGA;

                7'h08: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                7'h09: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                7'h0A: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                7'h0B: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;

                7'h10: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_1080P;
                7'h11: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_960P;
                7'h12: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_480P;
                7'h13: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_VGA;

                7'h20: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                7'h21: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                7'h22: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                7'h23: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;

                7'h40: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                7'h41: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                7'h42: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                7'h43: _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
            endcase
        end

        hdmiVideoConfig_reg <= _hdmiVideoConfig_reg;
    end

endmodule