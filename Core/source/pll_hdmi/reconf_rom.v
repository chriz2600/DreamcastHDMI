module reconf_rom (
    input clock,
    input [7:0] address,
    input read_ena,
    output q,
    output reconfig,

    input rdempty,
    input [7:0] fdata,
    output reg rdreq,
    output reg trigger_read,
    output reg forceVGAMode,
    output DCVideoConfig dcVideoConfig
);

reg _read_ena = 0;
reg q_reg;
reg q_reg_2;
reg doReconfig;
reg doReconfig_2;
reg doReconfig_3;

reg [7:0] fdata_req;

assign q = q_reg_2;
assign reconfig = doReconfig_3;

`include "../config/dc_config.v"

initial begin
    dcVideoConfig <= DC_VIDEO_CONFIG_1080P;
end

always @(posedge clock) begin
    _read_ena <= read_ena;

    if (_read_ena && ~read_ena) begin
        doReconfig <= 1;
    end else begin
        doReconfig <= 0;
    end

    if (~rdempty) begin
        rdreq <= 1'b1;
    end else begin
        rdreq <= 1'b0;
    end

    if (rdreq) begin
        fdata_req <= fdata;
        trigger_read <= 1'b1;
        forceVGAMode <= fdata[7];
        case (fdata[6:0])
            7'h00: dcVideoConfig <= DC_VIDEO_CONFIG_1080P;
            7'h01: dcVideoConfig <= DC_VIDEO_CONFIG_960P;
            7'h02: dcVideoConfig <= DC_VIDEO_CONFIG_480P;
            7'h03: dcVideoConfig <= DC_VIDEO_CONFIG_VGA;
            
            7'h10: dcVideoConfig <= DC_VIDEO_CONFIG_240P_1080P;
            7'h11: dcVideoConfig <= DC_VIDEO_CONFIG_240P_960P;
            7'h12: dcVideoConfig <= DC_VIDEO_CONFIG_240P_480P;
            7'h13: dcVideoConfig <= DC_VIDEO_CONFIG_240P_VGA;
            
            7'h20: dcVideoConfig <= DC_VIDEO_CONFIG_480I;
            7'h21: dcVideoConfig <= DC_VIDEO_CONFIG_480I;
            7'h22: dcVideoConfig <= DC_VIDEO_CONFIG_480I;
            7'h23: dcVideoConfig <= DC_VIDEO_CONFIG_480I;

            7'h40: dcVideoConfig <= DC_VIDEO_CONFIG_576I;
            7'h41: dcVideoConfig <= DC_VIDEO_CONFIG_576I;
            7'h42: dcVideoConfig <= DC_VIDEO_CONFIG_576I;
            7'h43: dcVideoConfig <= DC_VIDEO_CONFIG_576I;
        endcase
    end else begin
        trigger_read <= 1'b0;
    end

    case (fdata_req[6:0])
        7'h00: begin `include "config/1080p.v" end
        7'h01: begin `include "config/960p.v" end
        7'h02: begin `include "config/480p.v" end
        7'h03: begin `include "config/VGA.v" end
        7'h10: begin `include "config/240p_1080p.v" end
        7'h11: begin `include "config/960p.v" end
        7'h12: begin `include "config/480p.v" end
        7'h13: begin `include "config/VGA.v" end
        default: begin `include "config/480p.v" end
    endcase

    // delay output, to match ROM based timing
    q_reg_2 <= q_reg;
    doReconfig_2 <= doReconfig;
    doReconfig_3 <= doReconfig_2;
end

endmodule
