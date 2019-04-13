//
//
// Copyright (c) 2012-2013 Ludvig Strigeus
// Copyright (c) 2017,2018 Sorgelig
//
// This program is GPL Licensed. See COPYING for the full license.
//
//
////////////////////////////////////////////////////////////////////////////////////////////////////////
`include "config.inc"

// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on

/* verilator lint_off DECLFILENAME */
/* verilator lint_off WIDTH */
/* verilator lint_off UNUSED */

module Hq2x_optimized
#(
    parameter LENGTH = 858, 
    parameter HALF_DEPTH = 0
) 
(
    input             clk,
    input             ce_x4,
    input  [DWIDTH:0] inputpixel,
    input             mono,
    input             disable_hq2x,
    input             reset_frame,
    input             reset_line,
    input       [1:0] read_y,
    input             hblank,
    output [DWIDTH:0] outpixel /*verilator public*/
);

localparam AWIDTH = $clog2(LENGTH)-1;
localparam DWIDTH = HALF_DEPTH ? 11 : 23;
localparam DWIDTH1 = DWIDTH+1;
localparam EXACT_BUFFER = 1;

`HQ_TABLE_TYPE [5:0] hqTable[256] = '{
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 47, 35, 23, 15, 55, 39,
    19, 19, 26, 58, 19, 19, 26, 58, 23, 15, 35, 35, 23, 15, 7,  35,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 55, 39, 23, 15, 51, 43,
    19, 19, 26, 58, 19, 19, 26, 58, 23, 15, 51, 35, 23, 15, 7,  43,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 61, 35, 35, 23, 61, 51, 35,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 35, 23, 15, 51, 35,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 61, 7,  35, 23, 61, 7,  43,
    19, 19, 26, 11, 19, 19, 26, 58, 23, 15, 51, 35, 23, 61, 7,  43,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 47, 35, 23, 15, 55, 39,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 35, 23, 15, 51, 35,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 55, 39, 23, 15, 51, 43,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 39, 23, 15, 7,  43,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 35, 23, 15, 51, 39,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 35, 23, 15, 7,  35,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 51, 35, 23, 15, 7,  43,
    19, 19, 26, 11, 19, 19, 26, 11, 23, 15, 7,  35, 23, 15, 7,  43
};

reg [23:0] Prev0, Prev1, Prev2, Curr0, Curr1, Curr2, Next0, Next1, Next2;
reg [23:0] A, B, D, F, G, H;
reg  [7:0] pattern, nextpatt;
reg  [1:0] cyc;

reg  curbuf;
reg  prevbuf = 0;
wire iobuf = !curbuf;

wire diff0, diff1;
DiffCheck diffcheck0(
    clk, 
    Curr1, 
    (cyc == 0) ? Prev0 : 
        (cyc == 1) ? Curr0 : 
            (cyc == 2) ? Prev2 : Next1, 
    diff0
);
DiffCheck diffcheck1(
    clk, 
    Curr1, 
    (cyc == 0) ? Prev1 : 
        (cyc == 1) ? Next0 : 
            (cyc == 2) ? Curr2 : Next2, 
    diff1
);

wire [7:0] new_pattern = {diff1, diff0, pattern[7:2]};

wire [23:0] X = (cyc == 0) ? A : (cyc == 1) ? Prev1 : (cyc == 2) ? Next1 : G;
wire [23:0] blend_result_pre;
Blend blender(clk, hqTable[nextpatt], disable_hq2x, Curr0_d, X_d, B_d, D_d, F_d, H_d, blend_result_pre);
reg [23:0] Curr0_d, X_d, B_d, D_d, F_d, H_d;

delayline #(
    .WIDTH(6*24),
    .CYCLES(4)
) blender_queue (
    .clock(clk),
    .in({ Curr0, X, B, D, F, H }),
    .out({ Curr0_d, X_d, B_d, D_d, F_d, H_d })
);

wire [DWIDTH:0] Curr20tmp;
wire     [23:0] Curr20 = HALF_DEPTH ? h2rgb(Curr20tmp) : Curr20tmp;
wire [DWIDTH:0] Curr21tmp;
wire     [23:0] Curr21 = HALF_DEPTH ? h2rgb(Curr21tmp) : Curr21tmp;

reg  [AWIDTH:0] wrin_addr2;
reg  [DWIDTH:0] wrpix;
reg             wrin_en;

function [23:0] h2rgb;
    input [11:0] v;
begin
    h2rgb = mono ? {v[7:0], v[7:0], v[7:0]} : {v[11:8],v[11:8],v[7:4],v[7:4],v[3:0],v[3:0]};
end
endfunction

function [11:0] rgb2h;
    input [23:0] v;
begin
    rgb2h = mono ? {4'b0000, v[23:20], v[19:16]} : {v[23:20], v[15:12], v[7:4]};
end
endfunction

hq2x_in #(.LENGTH(LENGTH), .DWIDTH(DWIDTH)) hq2x_in
(
    .clk(clk),

    .rdaddr(offs),
    .rdbuf0(prevbuf),
    .rdbuf1(curbuf),
    .q0(Curr20tmp),
    .q1(Curr21tmp),

    .wraddr(wrin_addr2),
    .wrbuf(iobuf),
    .data(wrpix),
    .wren(wrin_en)
);

reg     [AWIDTH+1:0] read_x /*verilator public*/;
reg     [AWIDTH+1:0] wrout_addr, wrout_addr_delayed;
reg                  wrout_en, wrout_en_delayed;
reg  [DWIDTH1*4-1:0] wrdata, wrdata_pre;
wire [DWIDTH1*4-1:0] outpixel_x4;
reg  [DWIDTH1*2-1:0] outpixel_x2;

delayline #(
    .WIDTH(AWIDTH+1+1+1),
    .CYCLES(22)
) wrdl (
    .clock(clk),
    .in({ wrout_en, wrout_addr }),
    .out({ wrout_en_delayed, wrout_addr_delayed })
);

assign outpixel = read_x[0] ? outpixel_x2[DWIDTH1*2-1:DWIDTH1] : outpixel_x2[DWIDTH:0];

hq2x_buf #(.NUMWORDS(EXACT_BUFFER ? LENGTH : LENGTH*2), .AWIDTH(AWIDTH+1), .DWIDTH(DWIDTH1*4-1)) hq2x_out
(
    .clock(clk),

    .rdaddress(EXACT_BUFFER ? read_x[AWIDTH+1:1] : {read_x[AWIDTH+1:1],read_y[1]}),
    .q(outpixel_x4),

    .data(wrdata),
    .wraddress(wrout_addr_delayed),
    .wren(wrout_en_delayed)
);

wire [DWIDTH:0] blend_result = HALF_DEPTH ? rgb2h(blend_result_pre) : blend_result_pre[DWIDTH:0];

reg [AWIDTH:0] offs /*verilator public*/;
always @(posedge clk) begin
    reg old_reset_line;
    reg old_reset_frame;

    wrout_en <= 0;
    wrin_en  <= 0;

    if(ce_x4) begin

        pattern <= new_pattern;
        if(read_x[0]) outpixel_x2 <= read_y[0] ? outpixel_x4[DWIDTH1*4-1:DWIDTH1*2] : outpixel_x4[DWIDTH1*2-1:0];

        if(~&offs) begin
            if (cyc == 1) begin
                Prev2 <= Curr20;
                Curr2 <= Curr21;
                Next2 <= HALF_DEPTH ? h2rgb(inputpixel) : inputpixel;
                wrpix <= inputpixel;
                wrin_addr2 <= offs;
                wrin_en <= 1;
            end

            case({cyc[1],^cyc})
                0: wrdata[DWIDTH1+DWIDTH:DWIDTH1]     <= blend_result;
                1: wrdata[DWIDTH1*3+DWIDTH:DWIDTH1*3] <= blend_result;
                2: wrdata[DWIDTH:0]                   <= blend_result;
                3: wrdata[DWIDTH1*2+DWIDTH:DWIDTH1*2] <= blend_result;
            endcase

            if(cyc==3) begin
                offs <= offs + 1'd1;
                wrout_addr <= EXACT_BUFFER ? offs : {offs, curbuf};
                wrout_en <= 1;
            end
        end

        if(cyc==0) begin
            nextpatt <= {new_pattern[7:6], new_pattern[3], new_pattern[5], new_pattern[2], new_pattern[4], new_pattern[1:0]};
        end else begin
            nextpatt <= {nextpatt[5], nextpatt[3], nextpatt[0], nextpatt[6], nextpatt[1], nextpatt[7], nextpatt[4], nextpatt[2]};
        end

        if(cyc==3) begin
            {A, G} <= {Prev0, Next0};
            {B, F, H, D} <= {Prev1, Curr2, Next1, Curr0};
            {Prev0, Prev1} <= {Prev1, Prev2};
            {Curr0, Curr1} <= {Curr1, Curr2};
            {Next0, Next1} <= {Next1, Next2};
        end else begin
            {B, F, H, D} <= {F, H, D, B};
        end

        cyc <= cyc + 1'b1;
        if(old_reset_line && ~reset_line) begin
            old_reset_frame <= reset_frame;
            offs <= 0;
            cyc <= 0;
            curbuf <= ~curbuf;
            prevbuf <= curbuf;
            {Prev0, Prev1, Prev2, Curr0, Curr1, Curr2, Next0, Next1, Next2} <= '0;
            if(old_reset_frame & ~reset_frame) begin
                curbuf <= 0;
                prevbuf <= 0;
            end
        end
        
        if(~hblank & ~&read_x) read_x <= read_x + 1'd1;
        if(hblank) read_x <= 0;

        old_reset_line  <= reset_line;
    end
end

endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////

module hq2x_in #(parameter LENGTH, parameter DWIDTH)
(
    input            clk,

    input [AWIDTH:0] rdaddr,
    input            rdbuf0, rdbuf1,
    output[DWIDTH:0] q0,q1,

    input [AWIDTH:0] wraddr,
    input            wrbuf,
    input [DWIDTH:0] data,
    input            wren
);

    localparam AWIDTH = $clog2(LENGTH)-1;
    wire  [DWIDTH:0] out[2];
    assign q0 = out[rdbuf0];
    assign q1 = out[rdbuf1];

    hq2x_buf #(.NUMWORDS(LENGTH), .AWIDTH(AWIDTH), .DWIDTH(DWIDTH)) buf0(clk,data,rdaddr,wraddr,wren && (wrbuf == 0),out[0]);
    hq2x_buf #(.NUMWORDS(LENGTH), .AWIDTH(AWIDTH), .DWIDTH(DWIDTH)) buf1(clk,data,rdaddr,wraddr,wren && (wrbuf == 1),out[1]);
endmodule

module hq2x_buf #(parameter NUMWORDS, parameter AWIDTH, parameter DWIDTH)
(
    input                   clock,
    input        [DWIDTH:0] data /*verilator public*/,
    input        [AWIDTH:0] rdaddress /*verilator public*/,
    input        [AWIDTH:0] wraddress /*verilator public*/,
    input                   wren /*verilator public*/,
    output logic [DWIDTH:0] q /*verilator public*/
);

(* max_depth = 1024 *) (* ramstyle = "no_rw_check" *) logic [DWIDTH:0] ram[0:NUMWORDS-1];

always_ff@(posedge clock) begin
    if(wren) ram[wraddress] <= data;
    q <= ram[rdaddress];
end

endmodule

////////////////////////////////////////////////////////////////////////////////////////////////////////

module DiffCheck
(
    input clock,
    input [23:0] rgb1,
    input [23:0] rgb2,
    output reg result
);

    reg [7:0] r, g, b, r_q, g_q, b_q;
    reg [8:0] t, gx, u, t_q, gx_q;
    reg [9:0] y, v;

    // if y is inside (-96..96)
    reg y_inside, u_inside, v_inside;

    always_ff @(posedge clock) begin
        // stage 1
        r <= rgb1[7:1]   - rgb2[7:1];
        g <= rgb1[15:9]  - rgb2[15:9];
        b <= rgb1[23:17] - rgb2[23:17];

        // stage 2
        t <= $signed(r) + $signed(b);
        gx <= { g[7], g };
        { r_q, g_q, b_q } <= { r, g, b };

        // stage 3
        y <= $signed(t) + $signed(gx);
        u <= $signed(r_q) - $signed(b_q);
        v <= $signed({g_q, 1'b0}) - $signed(t);

        // stage 4
        // if y is inside (-96..96)
        y_inside <= (y < 10'h60 || y >= 10'h3a0);
        // if u is inside (-16, 16)
        u_inside <= (u < 9'h10 || u >= 9'h1f0);
        // if v is inside (-24, 24)
        v_inside <= (v < 10'h18 || v >= 10'h3e8);

        // stage 5
        result <= !(y_inside && u_inside && v_inside);
    end
endmodule

module InnerBlend
(
    input clock,
    input  [8:0] Op,
    input  [7:0] A,
    input  [7:0] B,
    input  [7:0] C,
    output reg [7:0] O
);

    function  [10:0] mul8x3;
        input   [7:0] op1;
        input   [2:0] op2;
    begin
        mul8x3 = 11'd0;
        if(op2[0]) mul8x3 = mul8x3 + op1;
        if(op2[1]) mul8x3 = mul8x3 + {op1, 1'b0};
        if(op2[2]) mul8x3 = mul8x3 + {op1, 2'b00};
    end
    endfunction

    reg OpOnes;
    reg [10:0] Amul, Bmul, Cmul;
    reg [10:0] At, Bt, Ct;
    reg [11:0] Res;

    reg [8:0] Op_q, Op_q_q, Op_q_q_q, Op_q_q_q_q;
    reg [7:0] A_q, A_q_q, A_q_q_q, A_q_q_q_q, B_q, B_q_q, C_q, C_q_q;

    always_ff @(posedge clock) begin
        // stage 1
        { Op_q, A_q, B_q, C_q } <= { Op, A, B, C };

        // stage 2
        OpOnes <= Op_q[4];
        Amul <= mul8x3(A_q, Op_q[7:5]);
        Bmul <= mul8x3(B_q, {Op_q[3:2], 1'b0});
        Cmul <= mul8x3(C_q, {Op_q[1:0], 1'b0});
        { Op_q_q, A_q_q, B_q_q, C_q_q } <= { Op_q, A_q, B_q, C_q };

        // stage 3
        At <=  Amul;
        Bt <= (OpOnes == 0) ? Bmul : {3'b0, B_q_q};
        Ct <= (OpOnes == 0) ? Cmul : {3'b0, C_q_q};
        { Op_q_q_q, A_q_q_q } <= { Op_q_q, A_q_q };

        // stage 4
        Res <= {At, 1'b0} + Bt + Ct;
        { Op_q_q_q_q, A_q_q_q_q } <= { Op_q_q_q, A_q_q_q };

        // stage 5
        O <= Op_q_q_q_q[8] ? A_q_q_q_q : Res[11:4];
    end
endmodule

module Blend
(
    input clock,
    input   [5:0] rule,
    input         disable_hq2x,
    input  [23:0] E,
    input  [23:0] A,
    input  [23:0] B,
    input  [23:0] D,
    input  [23:0] F,
    input  [23:0] H,
    output reg [23:0] Result
);
    reg [23:0] E_reg, A_reg, B_reg, D_reg, F_reg, H_reg;
    reg [23:0] E_reg_d, A_reg_d, B_reg_d, D_reg_d, F_reg_q, H_reg_d;
    reg [23:0] Input1, Input2, Input3;
    reg [23:0] res_out, res_out_q, res_out_q_q, res_out_q_q_q, res_out_q_q_q_q;
    reg [1:0] input_ctrl;
    reg [8:0] op, op_q;
    reg [5:0] rule_reg, rule_1, rule_2, rule_3, rule_d;
    localparam BLEND0 = 9'b1_xxx_x_xx_xx; // 0: A
    localparam BLEND1 = 9'b0_110_0_10_00; // 1: (A * 12 + B * 4) >> 4
    localparam BLEND2 = 9'b0_100_0_10_10; // 2: (A * 8 + B * 4 + C * 4) >> 4
    localparam BLEND3 = 9'b0_101_0_10_01; // 3: (A * 10 + B * 4 + C * 2) >> 4
    localparam BLEND4 = 9'b0_110_0_01_01; // 4: (A * 12 + B * 2 + C * 2) >> 4
    localparam BLEND5 = 9'b0_010_0_11_11; // 5: (A * 4 + (B + C) * 6) >> 4
    localparam BLEND6 = 9'b0_111_1_xx_xx; // 6: (A * 14 + B + C) >> 4
    localparam AB = 2'b00;
    localparam AD = 2'b01;
    localparam DB = 2'b10;
    localparam BD = 2'b11;
    wire is_diff;
    DiffCheck diff_checker(clock, rule_reg[1] ? B_reg : H_reg, rule_reg[0] ? D_reg : F_reg, is_diff);

    delayline #(
        .WIDTH(6*24),
        .CYCLES(5)
    ) delay1 (
        .clock(clock),
        .in({ E_reg, A_reg, B_reg, D_reg, F_reg, H_reg }),
        .out({ E_reg_d, A_reg_d, B_reg_d, D_reg_d, F_reg_q, H_reg_d })
    );
    delayline #(
        .WIDTH(6),
        .CYCLES(4)
    ) delay2 (
        .clock(clock),
        .in(rule_reg),
        .out(rule_d)
    );

    always_ff @(posedge clock) begin
        { rule_reg, E_reg, A_reg, B_reg, D_reg, F_reg, H_reg } <= { rule, E, A, B, D, F, H };

        case({!is_diff, rule_d[5:2]})
            1,17:  {op, input_ctrl} <= {BLEND1, AB};
            2,18:  {op, input_ctrl} <= {BLEND1, DB};
            3,19:  {op, input_ctrl} <= {BLEND1, BD};
            4,20:  {op, input_ctrl} <= {BLEND2, DB};
            5,21:  {op, input_ctrl} <= {BLEND2, AB};
            6,22:  {op, input_ctrl} <= {BLEND2, AD};

             8: {op, input_ctrl} <= {BLEND0, 2'bxx};
             9: {op, input_ctrl} <= {BLEND0, 2'bxx};
            10: {op, input_ctrl} <= {BLEND0, 2'bxx};
            11: {op, input_ctrl} <= {BLEND1, AB};
            12: {op, input_ctrl} <= {BLEND1, AB};
            13: {op, input_ctrl} <= {BLEND1, AB};
            14: {op, input_ctrl} <= {BLEND1, DB};
            15: {op, input_ctrl} <= {BLEND1, BD};

            24: {op, input_ctrl} <= {BLEND2, DB};
            25: {op, input_ctrl} <= {BLEND5, DB};
            26: {op, input_ctrl} <= {BLEND6, DB};
            27: {op, input_ctrl} <= {BLEND2, DB};
            28: {op, input_ctrl} <= {BLEND4, DB};
            29: {op, input_ctrl} <= {BLEND5, DB};
            30: {op, input_ctrl} <= {BLEND3, BD};
            31: {op, input_ctrl} <= {BLEND3, DB};
            default: {op, input_ctrl} <= {11{1'bx}};
        endcase

        // Setting op[8] effectively disables HQ2X because blend will always return E.
        if (disable_hq2x) op[8] <= 1;
    end

    // Generate inputs to the inner blender. Valid combinations.
    // 00: E A B
    // 01: E A D 
    // 10: E D B
    // 11: E B D

    InnerBlend inner_blend1(clock, op_q, Input1[7:0],   Input2[7:0],   Input3[7:0],   res_out[7:0]);
    InnerBlend inner_blend2(clock, op_q, Input1[15:8],  Input2[15:8],  Input3[15:8],  res_out[15:8]);
    InnerBlend inner_blend3(clock, op_q, Input1[23:16], Input2[23:16], Input3[23:16], res_out[23:16]);

    always_ff @(posedge clock) begin
        op_q <= op;
        Input1 <= E_reg_d;
        Input2 <= !input_ctrl[1] ? A_reg_d :
                  !input_ctrl[0] ? D_reg_d : B_reg_d;
        Input3 <= !input_ctrl[0] ? B_reg_d : D_reg_d;
        { Result, res_out_q_q_q_q, res_out_q_q_q, res_out_q_q, res_out_q } <= { res_out_q_q_q_q, res_out_q_q_q, res_out_q_q, res_out_q, res_out };
    end
endmodule
