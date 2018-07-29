module maple(
    input clk,
    input pin1,
    input pin5,
    output ControllerData controller_data
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
reg[3:0] pullup_osd;

ControllerData cdata_in = { 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0 };
ControllerData cdata_out = { 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0 };

initial begin
    pos <= 0;
    controller_packet_check <= 0;
    pullup_osd <= 0;
end

maple_in test(
    .rst(1'b0),
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

always @(posedge clk) begin
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
        pullup_osd <= 0;
        cdata_in <= { 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0, 1'b0 };

        // check for controller packet, to assign output data
        if (controller_packet_check == 4'b1111) begin
            cdata_out[11:1] <= cdata_in[11:1];
            if (/*cdata_out.trigger_osd ||*/ pullup_osd == 4'b1111) begin
                cdata_out.trigger_osd <= 1'b1;
            end
        end
    end
    // get maple bus data
    if (maple_data_produce) begin
        case (pos)
            0: begin
                if (maple_data == 8'h03) begin // 3 additional frames
                    controller_packet_check[0] <= 1'b1;
                end
            end
            1: begin
                // ignore sender address
            end
            2: begin
                if (maple_data == 8'h00) begin // receipient is dc
                    controller_packet_check[1] <= 1'b1;
                end
            end
            3: begin
                if (maple_data == 8'h08) begin // command is data reply
                    controller_packet_check[2] <= 1'b1;
                end
            end
            4: begin
                if (maple_data == 8'h01) begin // func is controller
                    controller_packet_check[3] <= 1'b1;
                end
            end
            8: begin
                if (maple_data == 8'hFF) begin // ltrigger must be completely engaged
                    pullup_osd[0] <= 1'b1;
                end
                cdata_in.ltrigger <= (maple_data == 8'hFF);
            end
            9: begin
                if (maple_data == 8'hFF) begin // rtrigger must be completely engaged
                    pullup_osd[1] <= 1'b1;
                end
                cdata_in.rtrigger <= (maple_data == 8'hFF);
            end
            10: begin
                if (maple_data == 8'b_1111_1011) begin // buttons[15:8] X pressed
                    pullup_osd[2] <= 1'b1;
                end
                cdata_in.y <= ~maple_data[1];
                cdata_in.x <= ~maple_data[2];
            end
            11: begin
                if (maple_data == 8'b_1111_0011) begin // buttons[7:0] A pressed, START pressed
                    pullup_osd[3] <= 1'b1;
                end
                cdata_in.b <= ~maple_data[1];
                cdata_in.a <= ~maple_data[2];
                cdata_in.start <= ~maple_data[3];
                cdata_in.up <= ~maple_data[4];
                cdata_in.down <= ~maple_data[5];
                cdata_in.left <= ~maple_data[6];
                cdata_in.right <= ~maple_data[7];
            end
            default: begin
                // ignored
            end
        endcase
        pos <= pos + 1'b1;
    end
end

assign maple_trigger_start = trigger_start;
assign controller_data = cdata_out;

endmodule