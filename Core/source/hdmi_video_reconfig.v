module hdmi_video_reconfig(
    input clock,
    input [7:0] data_in,
    output r2v_f,
    output HDMIVideoConfig hdmiVideoConfig
);

`ifdef std
    `include "config/std/hdmi_config.v"
`elsif hq2x
    `include "config/hq2x/hdmi_config.v"
`endif
`include "config/hdmi_config.v"


    reg [7:0] data_in_reg = 0;

    HDMIVideoConfig _hdmiVideoConfig_reg;
    HDMIVideoConfig hdmiVideoConfig_reg;
    reg _r2v_f_reg;
    reg r2v_f_reg;
    initial begin
        _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
        hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
        _r2v_f_reg <= 1'b1;
        r2v_f_reg <= 1'b1;
    end
    assign hdmiVideoConfig = hdmiVideoConfig_reg;
    assign r2v_f = r2v_f_reg;

    always @(posedge clock) begin
        data_in_reg <= data_in;

        if (data_in_reg != data_in) begin
            case (data_in[6:0])
                // RECONF
                7'h00: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_1080P;
                    _r2v_f_reg <= 1'b1;
                end
                7'h01: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_960P;
                    _r2v_f_reg <= 1'b1;
                end
                7'h02: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480P;
                    _r2v_f_reg <= 0;
                end
                7'h03: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_VGA;
                    _r2v_f_reg <= 0;
                end

                7'h04: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_288P;
                    _r2v_f_reg <= 0;
                end
                7'h05: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_288P;
                    _r2v_f_reg <= 0;
                end
                7'h06: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_288P;
                    _r2v_f_reg <= 0;
                end
                7'h07: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_288P;
                    _r2v_f_reg <= 0;
                end

                7'h08: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                    _r2v_f_reg <= 0;
                end
                7'h09: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                    _r2v_f_reg <= 0;
                end
                7'h0A: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                    _r2v_f_reg <= 0;
                end
                7'h0B: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576P;
                    _r2v_f_reg <= 0;
                end

                7'h10: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_1080P;
                    _r2v_f_reg <= 0;
                end
                7'h11: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_960P;
                    _r2v_f_reg <= 0;
                end
                7'h12: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_480P;
                    _r2v_f_reg <= 0;
                end
                7'h13: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_240P_VGA;
                    _r2v_f_reg <= 0;
                end

                7'h20: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                    _r2v_f_reg <= 0;
                end
                7'h21: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                    _r2v_f_reg <= 0;
                end
                7'h22: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                    _r2v_f_reg <= 0;
                end
                7'h23: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_480I;
                    _r2v_f_reg <= 0;
                end

                7'h40: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                    _r2v_f_reg <= 0;
                end
                7'h41: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                    _r2v_f_reg <= 0;
                end
                7'h42: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                    _r2v_f_reg <= 0;
                end
                7'h43: begin
                    _hdmiVideoConfig_reg <= HDMI_VIDEO_CONFIG_576I;
                    _r2v_f_reg <= 0;
                end
            endcase
        end

        hdmiVideoConfig_reg <= _hdmiVideoConfig_reg;
        r2v_f_reg <= _r2v_f_reg;
    end

endmodule