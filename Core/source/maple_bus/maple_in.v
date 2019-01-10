module maple_in(
    input rst,
    input clk,
    input pin1,
    input pin5,
    input oe,
    output active,
    output start_detected,
    output end_detected,
    input trigger_start,
    input trigger_end,
    output [7:0] fifo_data,
    output data_produce
);

    assign active = active_q;
    assign start_detected = start_detected_q;
    assign end_detected = end_detected_q;
    assign fifo_data = { shiftreg_q, in_p1_old_q };
    assign data_produce = produce;

    reg in_p1_d, in_p1_q;
    reg in_p5_d, in_p5_q;
    reg in_p1_old_d, in_p1_old_q;
    reg in_p5_old_d, in_p5_old_q;

    reg active_d, active_q;
    reg start_detected_d, start_detected_q;
    reg end_detected_d, end_detected_q;

    reg [6:0] shiftreg_d, shiftreg_q;
    reg produce;

    reg p1_edge, p1_value;
    reg p5_edge, p5_value;

    localparam MODE_IDLE = 0;
    localparam MODE_START = 1;
    localparam MODE_PHASE1_PRE = 2;
    localparam MODE_PHASE1 = 3;
    localparam MODE_PHASE2 = 4;
    localparam MODE_END = 5;

    reg [2:0] mode_d, mode_q;
    reg [2:0] cnt_d, cnt_q;

    always @(*) begin
        in_p1_d = pin1;
        in_p5_d = pin5;
        in_p1_old_d = in_p1_q;
        in_p5_old_d = in_p5_q;
        active_d = active_q;
        start_detected_d = start_detected_q;
        end_detected_d = end_detected_q;
        shiftreg_d = shiftreg_q;
        produce = 1'b0;

        p1_value = in_p1_old_q;
        p5_value = in_p5_old_q;
        p1_edge = in_p1_old_q && !in_p1_q;
        p5_edge = in_p5_old_q && !in_p5_q;

        mode_d = MODE_IDLE;
        cnt_d = 3'b0;

        if (trigger_start || trigger_end) begin
            active_d = trigger_start;
            start_detected_d = 1'b0;
            end_detected_d = 1'b0;
        end else if(oe) begin
            start_detected_d = 1'b0;
            end_detected_d = 1'b0;
        end else if(active) begin
            mode_d = mode_q;
            cnt_d = cnt_q;
            case (mode_q)
                MODE_PHASE1, MODE_PHASE1_PRE: begin
                    if (p5_edge && p1_value && cnt_q == 0) begin
                        if (mode_q == MODE_PHASE1_PRE) begin
                            mode_d = MODE_PHASE1;
                        end else begin
                            mode_d = MODE_END;
                        end
                    end else if (p1_edge) begin
                        shiftreg_d = { shiftreg_q[5:0], p5_value };
                        mode_d = MODE_PHASE2;
                    end else if (p5_edge) begin
                        // Error
                    end
                end
            MODE_PHASE2: begin
                if (p5_edge) begin
                        shiftreg_d = { shiftreg_q[5:0], p1_value };
                        mode_d = MODE_PHASE1;
                    if (cnt_q == 3) begin
                        cnt_d = 3'b0;
                        produce = 1'b1;
                    end else begin
                        cnt_d = cnt_q + 1'b1;
                    end
                end else if (p5_edge) begin
                    // Error
                end
            end
            MODE_START: begin
                if (p1_value) begin
                    cnt_d = 3'b0;
                    if (p5_value && cnt_q == 4) begin
                        start_detected_d = 1'b1;
                        mode_d = MODE_PHASE1_PRE;
                    end else begin
                        mode_d = MODE_IDLE;
                    end
                end else if (p5_edge) begin
                    if (cnt_q < 7)
                        cnt_d = cnt_q + 1'b1;
                end
            end
            MODE_END: begin
                if (p5_value) begin
                    cnt_d = 3'b0;
                    mode_d = MODE_IDLE;
                    if (p1_value && cnt_q == 2) begin
                        end_detected_d = 1'b1;
                        active_d = 1'b0;
                    end
                end else if (p1_edge) begin
                    if (cnt_q < 7)
                        cnt_d = cnt_q + 1'b1;
                end
            end
            MODE_IDLE: begin
                if (p1_edge && p5_value) begin
                    mode_d = MODE_START;
                end else if(p5_edge && p1_value) begin
                    mode_d = MODE_END;
                end
            end
            endcase
        end
    end

    always @(posedge clk or posedge rst) begin
        if (rst) begin
            in_p1_q <= 1'b1;
            in_p5_q <= 1'b1;
            in_p1_old_q <= 1'b1;
            in_p5_old_q <= 1'b1;
            start_detected_q <= 1'b0;
            end_detected_q <= 1'b0;
            active_q <= 1'b0;
            shiftreg_q <= 7'b0;
            mode_q <= MODE_IDLE;
            cnt_q <= 3'b0;
        end else begin
            in_p1_q <= in_p1_d;
            in_p5_q <= in_p5_d;
            in_p1_old_q <= in_p1_old_d;
            in_p5_old_q <= in_p5_old_d;
            start_detected_q <= start_detected_d;
            end_detected_q <= end_detected_d;
            active_q <= active_d;
            shiftreg_q <= shiftreg_d;
            mode_q <= mode_d;
            cnt_q <= cnt_d;
        end // else: !if(rst)
    end // always @ (posedge clk)

endmodule // maple_in
