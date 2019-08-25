module dc_video_reconfig(
    input clock,
    input [7:0] data_in,
    output DCVideoConfig dcVideoConfig,
    output forceVGAMode
);

`ifdef std
    `include "config/std/dc_config.v"
`elsif hq2x
    `include "config/hq2x/dc_config.v"
`endif
`include "config/dc_config.v"

    reg [7:0] data_in_reg = 0;
    reg forceVGAMode_reg;

    DCVideoConfig dcVideoConfig_reg;
    initial begin
        dcVideoConfig_reg <= DC_VIDEO_CONFIG_1080P;
    end
    assign dcVideoConfig = dcVideoConfig_reg;
    assign forceVGAMode = forceVGAMode_reg;

    always @(posedge clock) begin
        data_in_reg <= data_in;

        if (data_in_reg != data_in) begin
            forceVGAMode_reg <= data_in[7];
            case (data_in[6:0])
                // RECONF
                7'h00: dcVideoConfig_reg <= DC_VIDEO_CONFIG_1080P;
                7'h01: dcVideoConfig_reg <= DC_VIDEO_CONFIG_960P;
                7'h02: dcVideoConfig_reg <= DC_VIDEO_CONFIG_480P;
                7'h03: dcVideoConfig_reg <= DC_VIDEO_CONFIG_VGA;

                7'h04: dcVideoConfig_reg <= DC_VIDEO_CONFIG_288P;
                7'h05: dcVideoConfig_reg <= DC_VIDEO_CONFIG_288P;
                7'h06: dcVideoConfig_reg <= DC_VIDEO_CONFIG_288P;
                7'h07: dcVideoConfig_reg <= DC_VIDEO_CONFIG_288P;

                7'h08: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576P;
                7'h09: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576P;
                7'h0A: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576P;
                7'h0B: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576P;

                7'h10: dcVideoConfig_reg <= DC_VIDEO_CONFIG_240P_1080P;
                7'h11: dcVideoConfig_reg <= DC_VIDEO_CONFIG_240P_960P;
                7'h12: dcVideoConfig_reg <= DC_VIDEO_CONFIG_240P_480P;
                7'h13: dcVideoConfig_reg <= DC_VIDEO_CONFIG_240P_VGA;
                
                7'h20: dcVideoConfig_reg <= DC_VIDEO_CONFIG_480I;
                7'h21: dcVideoConfig_reg <= DC_VIDEO_CONFIG_480I;
                7'h22: dcVideoConfig_reg <= DC_VIDEO_CONFIG_480I;
                7'h23: dcVideoConfig_reg <= DC_VIDEO_CONFIG_480I;

                7'h40: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576I;
                7'h41: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576I;
                7'h42: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576I;
                7'h43: dcVideoConfig_reg <= DC_VIDEO_CONFIG_576I;
            endcase
        end
    end
endmodule