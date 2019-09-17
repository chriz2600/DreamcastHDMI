module maple(
    input clk,
    input reset,
    input pin1,
    input pin5,
    output ControllerData controller_data,
    output KeyboardData keyboard_data
);

wire maple_active;
wire maple_start_detected;
wire maple_end_detected;
wire maple_data_produce;

wire maple_trigger_start;
wire maple_trigger_end;

wire [7:0] maple_data;

reg [7:0] pos;
reg trigger_start = 0;
reg triggered = 0;

reg[3:0] controller_packet_check;
reg[3:0] keyboard_packet_check;
reg[3:0] pullup_osd;
reg[3:0] trig_def_res;

ControllerData cdata_in = 0;
ControllerData cdata_out = 0;

KeyboardData keydata_in = 0;
KeyboardData keydata_out = 0;

initial begin
    pos <= 0;
    controller_packet_check <= 0;
    keyboard_packet_check <= 0;
    pullup_osd <= 0;
    trig_def_res <= 0;
end

maple_in test(
    .rst(reset),
    .clk(clk),
    .pin1(pin1),
    .pin5(pin5),
    .oe(1'b0),
    .active(maple_active),
    .start_detected(maple_start_detected),
    .end_detected(maple_end_detected),
    .trigger_start(maple_trigger_start),
    .trigger_end(maple_trigger_end),
    .fifo_data(maple_data),
    .data_produce(maple_data_produce)
);

always @(posedge clk or posedge reset) begin
    if (reset) begin
        // reset values
        triggered <= 0;
        pos <= 0;
        controller_packet_check <= 0;
        keyboard_packet_check <= 0;
        pullup_osd <= 0;
        trig_def_res <= 0;
        cdata_in <= 0;
        keydata_in <= 0;
    end else begin
        // re-trigger read loop
        if (!triggered) begin
            trigger_start <= 1;
            triggered <= 1;
        end else begin
            trigger_start <= 0;
        end
        // re-trigger read loop on package end
        if (maple_end_detected) begin
            // reset values
            triggered <= 0;
            pos <= 0;
            controller_packet_check <= 0;
            keyboard_packet_check <= 0;
            pullup_osd <= 0;
            trig_def_res <= 0;
            cdata_in <= 0;
            keydata_in <= 0;

            // check for controller packet, to assign output data
            if (controller_packet_check == 4'b1111) begin
                cdata_out[12:2] <= cdata_in[12:2];
                cdata_out.trigger_osd <= (pullup_osd == 4'b1111);
                cdata_out.trigger_default_resolution <= (trig_def_res == 4'b1111);
                cdata_out.valid_packet <= 1'b1;
            end
            if (keyboard_packet_check == 4'b1111) begin
                keydata_out[63:0] <= keydata_in[63:0];
                keydata_out.valid_packet <= 1'b1;
            end
        end
        // get maple bus data
        if (maple_data_produce) begin
            case (pos)
                0: begin
                    if (maple_data == 8'h03) begin // 3 additional frames
                        controller_packet_check[0] <= 1'b1;
                        keyboard_packet_check[0] <= 1'b1;
                    end
                end
                1: begin
                    // ignore sender address
                end
                2: begin
                    if (maple_data == 8'h00) begin // receipient is dc
                        controller_packet_check[1] <= 1'b1;
                        keyboard_packet_check[1] <= 1'b1;
                    end
                end
                3: begin
                    if (maple_data == 8'h08) begin // command is data reply
                        controller_packet_check[2] <= 1'b1;
                        keyboard_packet_check[2] <= 1'b1;
                    end
                end
                4: begin
                    if (maple_data == 8'h01) begin // func is controller
                        controller_packet_check[3] <= 1'b1;
                    end else if (maple_data == 8'h40) begin // func is keyboard
                        keyboard_packet_check[3] <= 1'b1;
                    end
                end
                8: begin
                    if (maple_data == 8'hFF) begin // ltrigger must be completely engaged
                        pullup_osd[0] <= 1'b1;
                        trig_def_res[0] <= 1'b1;
                    end
                    cdata_in.ltrigger <= (maple_data == 8'hFF);
                    keydata_in.key2 <= maple_data;
                end
                9: begin
                    if (maple_data == 8'hFF) begin // rtrigger must be completely engaged
                        pullup_osd[1] <= 1'b1;
                        trig_def_res[1] <= 1'b1;
                    end
                    cdata_in.rtrigger <= (maple_data == 8'hFF);
                    keydata_in.key1 <= maple_data;
                end
                10: begin
                    if (maple_data == 8'b_1111_1011) begin // buttons[15:8] X pressed
                        pullup_osd[2] <= 1'b1;
                    end else if (maple_data == 8'b_1111_1101) begin // buttons[15:8] Y pressed
                        trig_def_res[2] <= 1'b1;
                    end else if (maple_data == 8'b_1111_1100) begin // buttons[15:8] Z pressed, Y pressed
                        pullup_osd[1:0] <= 2'b11;
                    end
                    cdata_in.y <= ~maple_data[1];
                    cdata_in.x <= ~maple_data[2];
                    keydata_in.leds <= maple_data;
                    // map arcade stick Z to ltrigger/cancel
                    cdata_in.ltrigger <= ~maple_data[0] | cdata_in.ltrigger;
                end
                11: begin
                    if (maple_data == 8'b_1111_0011) begin // buttons[7:0] A pressed, START pressed
                        pullup_osd[3] <= 1'b1;
                    end else if (maple_data == 8'b_1111_0101) begin // buttons[7:0] B pressed, START pressed
                        trig_def_res[3] <= 1'b1;
                    end else if (maple_data == 8'b_1111_0100) begin // buttons[7:0] C pressed, B pressed, START pressed
                        pullup_osd[3:2] <= 2'b11;
                    end
                    cdata_in.b <= ~maple_data[1];
                    cdata_in.a <= ~maple_data[2];
                    cdata_in.start <= ~maple_data[3];
                    cdata_in.up <= ~maple_data[4];
                    cdata_in.down <= ~maple_data[5];
                    cdata_in.left <= ~maple_data[6];
                    cdata_in.right <= ~maple_data[7];
                    keydata_in.shiftcode <= maple_data;
                    // map arcade stick C to rtrigger/ok
                    cdata_in.rtrigger <= ~maple_data[0] | cdata_in.rtrigger;
                end
                12: begin
                    keydata_in.key6 <= maple_data;
                end
                13: begin
                    keydata_in.key5 <= maple_data;
                end
                14: begin
                    keydata_in.key4 <= maple_data;
                end
                15: begin
                    keydata_in.key3 <= maple_data;
                end
                default: begin
                    // ignored
                end
            endcase
            pos <= pos + 1'b1;
        end
    end
end

assign maple_trigger_start = trigger_start;
assign controller_data = cdata_out;
assign keyboard_data = keydata_out;

endmodule