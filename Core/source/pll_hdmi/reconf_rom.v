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
    output reg resetPLL,
    output reg generate_video,
    output reg generate_timing,
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
        resetPLL <= fdata[6]; // this value is only a trigger and is being reset immediately in next cycle
        generate_video <= fdata[5];
        generate_timing <= fdata[4];
        case (fdata[3:0])
            0: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_1080P;
            end
            1: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_960P;
            end
            2: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_480P;
            end
            3: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_VGA;
            end
            4: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_240Px3;
            end
            5: begin
                dcVideoConfig <= DC_VIDEO_CONFIG_240Px4;
            end
        endcase
    end else begin
        trigger_read <= 1'b0;
        resetPLL <= 1'b0;
    end

    case (fdata_req[3:0])
        0: begin
            `include "config/1080p.v"
        end
        1: begin
            `include "config/960p.v"
        end
        2: begin
            `include "config/480p.v"
        end
        3: begin
            `include "config/VGA.v"
        end
        4: begin
            `include "config/240p_x3.v"
        end
        5: begin
            `include "config/960p.v"
        end
    endcase

    // delay output, to match ROM based timing
    q_reg_2 <= q_reg;
    doReconfig_2 <= doReconfig;
    doReconfig_3 <= doReconfig_2;
end

endmodule
