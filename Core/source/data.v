module data(
    input clock,
    input reset,
    input [11:0] indata,
    input _hsync,
    input _vsync,
    input line_doubler,
    input generate_video,
    input generate_timing,
    input [23:0] conf240p,
    input nonBlackPixelReset,
    
    output [7:0] red,
    output [7:0] green,
    output [7:0] blue,
    
    output [11:0] counterX,
    output [11:0] counterY,
    output add_line,
    output is_pal,
    output resync,
    output [23:0] pinok,
    output [23:0] timingInfo,
    output [23:0] rgbData,

    output [11:0] nonBlackPos1,
    output [11:0] nonBlackPos2,

    output reg [23:0] color_space_explorer,

    output reg force_generate
);

    reg hsync_reg;
    reg vsync_reg;

    reg [31:0] vsync_reg_store = 0;

    reg [7:0] red_reg_buf;
    reg [7:0] red_reg;
    reg [7:0] green_reg_buf;
    reg [7:0] green_reg;
    reg [7:0] blue_reg;

    reg [11:0] raw_counterX = 0;
    reg [11:0] raw_counterX_reg = 0;
    reg [11:0] counterX_reg;
    reg [11:0] counterX_reg_q;

    reg [11:0] raw_counterY = 0;
    reg [11:0] raw_counterY_reg = 0;
    reg [11:0] counterY_reg;
    reg [11:0] counterY_reg_q;

    reg [9:0] VISIBLE_AREA_HSTART;
    reg [9:0] VISIBLE_AREA_VSTART;
    reg [9:0] VISIBLE_AREA_WIDTH;
    reg [9:0] VISIBLE_AREA_HEIGHT;
    
    reg add_line_reg = 0;
    reg is_pal_reg = 0;
    reg resync_reg = 1;

    reg [10:0] pinok1 = 0;
    reg [10:0] pinok2 = 0;
    reg [10:0] pinok1_reg = 0;
    reg [10:0] pinok2_reg = 0;

    reg [23:0] rgbData_buf = 0;
    reg [23:0] rgbData_reg = 0;

    reg [11:0] nonBlackPos1_reg;
    reg [11:0] nonBlackPos2_reg;
    reg [11:0] nonBlackPos1_reg_q;
    reg [11:0] nonBlackPos2_reg_q;

    reg [23:0] color_space_explorer_reg;

    initial begin
        raw_counterX_reg = 0;
        raw_counterY_reg = 0;
        add_line_reg = 0;
        is_pal_reg = 0;
        resync_reg = 1;
        force_generate = 0;
        pinok1 = 0;
        pinok2 = 0;
        nonBlackPos1_reg = 12'b111111111111;
        nonBlackPos2_reg = 12'b000000000000;
        color_space_explorer = 24'd0;
        color_space_explorer_reg = 24'd0;
    end

    always @(*) begin
        if (line_doubler) begin
            if (is_pal && add_line) begin // 288p
                VISIBLE_AREA_HSTART = 10'd257 - `OFFSET_V_AREA;
                VISIBLE_AREA_VSTART = 10'd24;
                VISIBLE_AREA_WIDTH  = 10'd720 + `OFFSET_V_AREA;
                VISIBLE_AREA_HEIGHT = 10'd600;
            end else if (add_line) begin // 240p
                VISIBLE_AREA_HSTART = 10'd257 - `OFFSET_V_AREA;
                VISIBLE_AREA_VSTART = 10'd18;
                VISIBLE_AREA_WIDTH  = 10'd720 + `OFFSET_V_AREA;
                VISIBLE_AREA_HEIGHT = 10'd504;
            end else if (is_pal) begin // 576i
                VISIBLE_AREA_HSTART = 10'd279 - `OFFSET_V_AREA;
                VISIBLE_AREA_VSTART = 10'd22;
                VISIBLE_AREA_WIDTH  = 10'd720 + `OFFSET_V_AREA;
                VISIBLE_AREA_HEIGHT = 10'd600;
            end else begin // 480i
                VISIBLE_AREA_HSTART = 10'd257 - `OFFSET_V_AREA; // OK
                VISIBLE_AREA_VSTART = 10'd18;
                VISIBLE_AREA_WIDTH  = 10'd720 + `OFFSET_V_AREA;
                VISIBLE_AREA_HEIGHT = 10'd504;
            end
        end else begin // VGA
            VISIBLE_AREA_HSTART = 10'd265 - `OFFSET_V_AREA; // OK, for commercial games
            VISIBLE_AREA_VSTART = 10'd40;
            VISIBLE_AREA_WIDTH  = 10'd720 + `OFFSET_V_AREA;
            VISIBLE_AREA_HEIGHT = 10'd480;
        end
    end

    `define RAW_WIDTH 1716
    `define RAW_HEIGHT 525
    `define HORIZONTAL_OFFSET 52

    always @(posedge clock or posedge reset) begin
        if (reset) begin
            resync_reg <= 1;
            vsync_reg_store <= 0;
            force_generate <= 0;
            pinok1 <= 0;
            pinok2 <= 0;
            color_space_explorer <= 24'b0;
            color_space_explorer_reg <= 24'b0;
        end else begin
            if (force_generate || generate_timing) begin
                if (raw_counterX_reg < `RAW_WIDTH - 1) begin
                    raw_counterX_reg <= raw_counterX_reg + 1'b1;
                end else begin
                    raw_counterX_reg <= 0;

                    if (raw_counterY_reg < `RAW_HEIGHT - 1) begin
                        raw_counterY_reg <= raw_counterY_reg + 1'b1;
                    end else begin
                        resync_reg <= 0;
                        raw_counterY_reg <= 0;
                    end
                end
            end else begin
                hsync_reg <= _hsync;
                vsync_reg <= _vsync;

                // 
                if (vsync_reg && _vsync && hsync_reg && _hsync && (& indata)) begin
                    vsync_reg_store <= vsync_reg_store + 1;

                    if (vsync_reg_store == 32'd_108_000_000) begin
                        force_generate <= 1'b1;
                    end
                end else begin
                    vsync_reg_store <= 0;
                end

                // reset horizontal raw counter on hsync
                if (hsync_reg && !_hsync) begin
                    raw_counterX <= raw_counterX_reg;
                    raw_counterX_reg <= 0;

                    // reset vertical raw counter on vsync
                    if (vsync_reg && !_vsync) begin
                        /*
                            Check, if actual line is the same as before,
                            as we have to send a resync to the hdmi output side
                            if not, to keep the vertical alignment :)
                        */
                        if (raw_counterY_reg == 262
                         || raw_counterY_reg == 524
                         || raw_counterY_reg == 312
                         || raw_counterY_reg == 624) begin
                            resync_reg <= 0;
                        end else begin
                            resync_reg <= 1;
                        end

                        // 240p/288p has only 263/313 lines per frame
                        if (raw_counterY_reg == 262
                         || raw_counterY_reg == 312) begin
                            add_line_reg <= 1'b1;
                        end else begin
                            add_line_reg <= 1'b0;
                        end

                        // PAL
                        if (raw_counterY_reg == 312
                         || raw_counterY_reg == 624) begin
                            is_pal_reg <= 1'b1;
                        end else begin
                            is_pal_reg <= 1'b0;
                        end

                        raw_counterY <= raw_counterY_reg;
                        raw_counterY_reg <= 0;
                    end else begin
                        raw_counterY_reg <= raw_counterY_reg + 1'b1;
                    end
                end else begin
                    raw_counterX_reg <= raw_counterX_reg + 1'b1;
                end
            end

            // recalculate counterX and counterY to match visible area
            if (raw_counterX_reg == VISIBLE_AREA_HSTART + (add_line ? conf240p[7:0] : line_doubler ? 0 : $signed(conf240p[15:8]))) begin
                counterX_reg <= 0;
                
                if (raw_counterY_reg == VISIBLE_AREA_VSTART) begin
                    counterY_reg <= 0;
                    // reset pinok test data on new frame
                    pinok1_reg <= pinok1;
                    pinok2_reg <= pinok2;
                    pinok1 <= 0;
                    pinok2 <= 0;
                    // store bits used once per frame
                    color_space_explorer <= color_space_explorer_reg;
                    color_space_explorer_reg <= 24'd0;
                end else begin
                    counterY_reg <= counterY_reg + 1'b1;
                end
            end else begin
                counterX_reg <= counterX_reg + raw_counterX_reg[0];
            end

            // store red and first half of green
            if (counterX_reg >= 0 && counterX_reg < VISIBLE_AREA_WIDTH 
             && counterY_reg >= 0 && counterY_reg < VISIBLE_AREA_HEIGHT) begin
                if (force_generate || generate_video) begin
                    // store values on even clock
                    if (~raw_counterX_reg[0]) begin
                        doOutputValue(counterX_reg, 8'd255);
                        // if (counterY_reg < 240) begin
                        //     doOutputValue(counterX_reg, ((~counterY_reg[7:0] / 32) * 32));
                        // end else begin
                        //     doOutputValue(counterX_reg, (((counterY_reg[7:0] + 16) / 32) * 32) + 15);
                        // end
                    end
                end else begin
                    checkPins();
                    // store values on even clock
                    if (raw_counterX_reg[0]) begin
                        red_reg_buf <= indata[11:4];
                        green_reg_buf[7:4] <= indata[3:0];
                    end else begin
                        // apply combined values of red, green, blue simultanesly
                        red_reg <= red_reg_buf;
                        green_reg <= { green_reg_buf[7:4], indata[11:8] };
                        blue_reg <= indata[7:0];
                        if (counterX_reg == 120 && counterY_reg == 120) begin
                            rgbData_buf <= { red_reg_buf, green_reg_buf[7:4], indata[11:8], indata[7:0] };
                        end
                        // mark bits used in frame
                        color_space_explorer_reg <= color_space_explorer_reg | { red_reg_buf, green_reg_buf[7:4], indata[11:8], indata[7:0] };
                    end
                end
            end else begin
                red_reg <= 8'd0;
                green_reg <= 8'd0;
                blue_reg <= 8'd0;
                rgbData_reg <= rgbData_buf;
            end

            counterX_reg_q <= counterX_reg;
            counterY_reg_q <= counterY_reg;

            // non black pixel detection
            if (nonBlackPixelReset) begin
                nonBlackPos1_reg <= 12'b111111111111;
                nonBlackPos2_reg <= 12'b000000000000;
                nonBlackPos1_reg_q <= 12'b111111111111;
                nonBlackPos2_reg_q <= 12'b000000000000;
            end else if (counterX_reg_q == VISIBLE_AREA_WIDTH - 1) begin
                nonBlackPos1_reg_q <= nonBlackPos1_reg;
                nonBlackPos2_reg_q <= nonBlackPos2_reg;
            end else if ({ red_reg, green_reg, blue_reg } != 24'd0) begin
                if (counterX_reg_q < nonBlackPos1_reg) begin
                    nonBlackPos1_reg <= counterX_reg_q;
                end else if (counterX_reg_q > nonBlackPos2_reg) begin
                    nonBlackPos2_reg <= counterX_reg_q;
                end
            end
        end
    end

    function get_fifth_bit(
        input[11:0] value
    );
        get_fifth_bit = value[5];
    endfunction

    assign nonBlackPos1 = nonBlackPos1_reg_q;
    assign nonBlackPos2 = nonBlackPos2_reg_q;
    assign counterX = counterX_reg_q;
    assign counterY = counterY_reg_q;
    assign red = red_reg;
    assign green = green_reg;
    assign blue = blue_reg;
    assign add_line = add_line_reg;
    assign is_pal = is_pal_reg;
    assign resync = resync_reg;
    assign pinok = { 2'b00, pinok1_reg, pinok2_reg };
    assign timingInfo = { raw_counterX, raw_counterY };
    assign rgbData = rgbData_reg;

    task doOutputValue;
        input [11:0] xpos;
        input [7:0] val;
        begin
            if (xpos >= `HORIZONTAL_OFFSET && xpos < 640 + `HORIZONTAL_OFFSET) begin
                if (xpos < 80 + `HORIZONTAL_OFFSET) begin
                    red_reg <= val;
                    green_reg <= 8'd0;
                    blue_reg <= 8'd0;
                end else if (xpos < 160 + `HORIZONTAL_OFFSET) begin
                    red_reg <= 8'd0;
                    green_reg <= val;
                    blue_reg <= 8'd0;
                end else if (xpos < 240 + `HORIZONTAL_OFFSET) begin
                    red_reg <= 8'd0;
                    green_reg <= 8'd0;
                    blue_reg <= val;
                end else if (xpos < 320 + `HORIZONTAL_OFFSET) begin
                    red_reg <= val;
                    green_reg <= val;
                    blue_reg <= val;
                end else if (xpos < 400 + `HORIZONTAL_OFFSET) begin
                    red_reg <= 8'd0;
                    green_reg <= 8'd0;
                    blue_reg <= 8'd0;
                end else if (xpos < 480 + `HORIZONTAL_OFFSET) begin
                    red_reg <= 8'd0;
                    green_reg <= val;
                    blue_reg <= val;
                end else if (xpos < 560 + `HORIZONTAL_OFFSET) begin
                    red_reg <= val;
                    green_reg <= val;
                    blue_reg <= 8'd0;
                end else begin
                    red_reg <= val;
                    green_reg <= 8'd0;
                    blue_reg <= val;
                end
            end else begin
                red_reg <= 8'd0;
                green_reg <= 8'd0;
                blue_reg <= 8'd0;
            end
        end
    endtask

    task checkPins;
        begin
            //////////////////////////////////////////////
            // check for pins being shorted
            if (indata[1:0] == 2'b10) pinok1[0] <= 1'b1;
            if (indata[2:1] == 2'b10) pinok1[1] <= 1'b1;
            if (indata[3:2] == 2'b10) pinok1[2] <= 1'b1;
            if (indata[4:3] == 2'b10) pinok1[3] <= 1'b1;
            if (indata[5:4] == 2'b10) pinok1[4] <= 1'b1;
            if (indata[6:5] == 2'b10) pinok1[5] <= 1'b1;
            if (indata[7:6] == 2'b10) pinok1[6] <= 1'b1;
            if (indata[8:7] == 2'b10) pinok1[7] <= 1'b1;
            if (indata[9:8] == 2'b10) pinok1[8] <= 1'b1;
            if (indata[10:9] == 2'b10) pinok1[9] <= 1'b1;
            if (indata[11:10] == 2'b10) pinok1[10] <= 1'b1;
            if (indata[1:0] == 2'b01) pinok2[0] <= 1'b1;
            if (indata[2:1] == 2'b01) pinok2[1] <= 1'b1;
            if (indata[3:2] == 2'b01) pinok2[2] <= 1'b1;
            if (indata[4:3] == 2'b01) pinok2[3] <= 1'b1;
            if (indata[5:4] == 2'b01) pinok2[4] <= 1'b1;
            if (indata[6:5] == 2'b01) pinok2[5] <= 1'b1;
            if (indata[7:6] == 2'b01) pinok2[6] <= 1'b1;
            if (indata[8:7] == 2'b01) pinok2[7] <= 1'b1;
            if (indata[9:8] == 2'b01) pinok2[8] <= 1'b1;
            if (indata[10:9] == 2'b01) pinok2[9] <= 1'b1;
            if (indata[11:10] == 2'b01) pinok2[10] <= 1'b1;
            //////////////////////////////////////////////
        end
    endtask

endmodule