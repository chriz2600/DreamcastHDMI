`include "config.inc"

module ADV7513(
    input clk,
    input reset,
    input hdmi_int,
    input VSYNC,
    input DE,

    inout sda,
    inout scl,
    input restart,
    output reg ready,
    output DebugData debugData_out,

    input HDMIVideoConfig hdmiVideoConfig
);

reg [6:0] i2c_chip_addr;
reg [7:0] i2c_reg_addr;
reg [7:0] i2c_value;
reg i2c_enable;
reg i2c_is_read;

wire [7:0] i2c_data;
wire i2c_done;
wire i2c_ack_error;

I2C I2C(
    .clk           (clk),
    .reset         (1'b1),

    .chip_addr     (i2c_chip_addr),
    .reg_addr      (i2c_reg_addr),
    .value         (i2c_value),
    .enable        (i2c_enable),
    .is_read       (i2c_is_read),

    .sda           (sda),
    .scl           (scl),

    .data          (i2c_data),
    .done          (i2c_done),
    .i2c_ack_error (i2c_ack_error),

    .divider       (hdmiVideoConfig.divider)
);

(* syn_encoding = "safe" *) 
reg [1:0] state;
reg [2:0] cmd_counter;
reg [5:0] subcmd_counter;

reg VSYNC_reg = 0;
reg DE_reg = 0;

DebugData debugData = { 8'h00, 8'h00, 10'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00, 8'h00 };

localparam CHIP_ADDR = 7'h39;
localparam  s_start  = 0,
            s_wait   = 1,
            s_wait_2 = 2,
            s_idle   = 3;
localparam  cs_init     = 3'd0,
            cs_init2    = 3'd1,
            cs_debug    = 3'd2,
            cs_pllcheck = 3'd3,
            cs_pwrup    = 3'd4,
            cs_pwrdown  = 3'd5,
            cs_ctsdebug = 3'd6,
            cs_ready    = 3'd7;
localparam  scs_start = 6'd0;

initial begin
    ready <= 0;
end

assign debugData_out = debugData;

always @ (posedge clk) begin

    if (restart) begin
        debugData.restart_count <= debugData.restart_count + 1'b1;
    end

    if (~reset) begin
        state <= s_start;
        cmd_counter <= cs_init;
        subcmd_counter <= scs_start;
        i2c_enable <= 1'b0;
        ready <= 0;
    end else begin
        VSYNC_reg <= VSYNC;
        DE_reg <= DE;
        case (state)
            
            s_start: begin
                if (i2c_done) begin
                    
                    case (cmd_counter)
                        cs_init: adv7513_bootstrap(cs_pwrdown);
                        cs_pwrdown: adv7513_link_powerdown(cs_init2);
                        cs_init2: adv7513_init(cs_pllcheck);
                        cs_pllcheck: adv7513_pllcheck(cs_pwrup);
                        cs_pwrup: adv7513_link_powerup(cs_ready);

                        cs_debug: adv7513_debug(cs_ready);
                        cs_ctsdebug: adv7513_ctscheck(cs_ready);

                        default: begin
                            cmd_counter <= cs_init;
                            subcmd_counter <= scs_start;
                            state <= s_idle;
                            ready <= 1;
                        end
                    endcase
                end
            end
            
            s_wait: begin
                state <= s_wait_2;
            end
            
            s_wait_2: begin
                i2c_enable <= 1'b0;
                
                if (i2c_done) begin
                    if (~i2c_ack_error) begin
                        subcmd_counter <= subcmd_counter + 1'b1;
                    end 
                    state <= s_start;
                end
            end

            s_idle: begin
                if (~hdmi_int) begin
                    ready <= 0;
                    state <= s_start;
                end else if (~DE_reg && DE) begin
                    cmd_counter <= cs_ctsdebug;
                    state <= s_start;
                end else if (~VSYNC_reg && VSYNC) begin
                    cmd_counter <= cs_debug;
                    state <= s_start;
                    debugData.frame_counter <= debugData.frame_counter + 1'b1;
                end

                if (debugData.frame_counter == 1023) begin
                    debugData.max_cts1_status <= 0;
                    debugData.max_cts2_status <= 0;
                    debugData.max_cts3_status <= 0;
                    debugData.summary_cts1_status <= 0;
                    debugData.summary_cts2_status <= 0;
                    debugData.summary_cts3_status <= 0;
                    debugData.summary_summary_cts3_status <= debugData.summary_cts3_status;
                    debugData.frame_counter <= 0;
                    debugData.test <= debugData.test + 1'b1;
                end
            end
            
        endcase
    end
end

task do_cts;
    output [7:0] cts_out;

    input [7:0] max_cts_in;
    output [7:0] max_cts_out;

    input [7:0] offset_cts_in;
    output [7:0] offset_cts_out;

    begin
        subcmd_counter <= subcmd_counter + 1'b1;
        cts_out <= i2c_data;
        if (i2c_data >= max_cts_in) begin
            max_cts_out <= i2c_data;
        end else begin
            max_cts_out <= max_cts_in;
        end
        calculate_offset(i2c_data, max_cts_in, offset_cts_in, offset_cts_out);
    end
endtask

task calculate_offset;
    input [7:0] cur;
    input [7:0] max;
    input [7:0] offset_cts_in;
    output [7:0] offset_cts_out;

    begin
        if (max > cur && (max - cur) > offset_cts_in) begin
            offset_cts_out <= max - cur;
        end else begin
            offset_cts_out <= offset_cts_in;
        end
    end
endtask

task write_i2c;
    input [6:0] t_chip_addr;
    input [15:0] t_data;

    begin
        i2c_chip_addr <= t_chip_addr;
        i2c_reg_addr  <= t_data[15:8];
        i2c_value     <= t_data[7:0];
        i2c_enable    <= 1'b1;
        i2c_is_read   <= 1'b0;
        state         <= s_wait;
    end
endtask

task read_i2c;
    input [6:0] t_chip_addr;
    input [7:0] t_addr;

    begin
        i2c_chip_addr <= t_chip_addr;
        i2c_reg_addr  <= t_addr;
        i2c_enable    <= 1'b1;
        i2c_is_read   <= 1'b1;
        state         <= s_wait;
    end
endtask

// ----------------------------------------------------------------

task adv7513_init;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_15_00); // [7:4]: I2S Sampling Frequency = 0b0000, 44.1kHz
                                                // [3:0]: Video Input ID = 0b0000, 24 bit RGB 4:4:4 (separate syncs)
            1: write_i2c(CHIP_ADDR, { 8'h_17, hdmiVideoConfig.adv_reg_17 });
                                                // [7]:   fixed = 0b0
                                                // [6]:   vsync polarity = 0b0, sync polarity pass through (sync adjust is off in 0x41)
                                                // [5]:   hsync polarity = 0b0, sync polarity pass through
                                                // [4:3]: reserved = 0b00
                                                // [2]:   4:2:2 to 4:4:4 interpolation style = 0b0, use zero order interpolation
                                                // [1]:   input video aspect ratio = 0b0, 4:3; 0b10 for 16:9
                                                // [0]:   DE generator = 0b0, disabled
            2: write_i2c(CHIP_ADDR, 16'h_16_30 | `OUTPUT_FMT);
                                                // [7]:   output format = 0b0, 4:4:4, (4:2:2, if OUTPUT_4_2_2 is set)
                                                // [6]:   reserved = 0b0
                                                // [5:4]: color depth = 0b11, 8bit
                                                // [3:2]: input style = 0b0, not valid
                                                // [1]:   ddr input edge = 0b0, falling edge
                                                // [0]:   output colorspace for blackimage = 0b0, RGB (YCbCr, if OUTPUT_4_2_2 is set)
            3: write_i2c(CHIP_ADDR, 16'h_55_00); // RGB in AVI InfoFrame
            4: write_i2c(CHIP_ADDR, 16'h_18_46); // [7]:   CSC enable = 0b0, disabled
                                                    // [6:5]: default = 0b10
                                                    // [4:0]: default = 0b00110
            5: write_i2c(CHIP_ADDR, 16'h_AF_06); // [7]:   HDCP enable = 0b0, disabled
                                                    // [6:5]: fixed = 0b00
                                                    // [4]:   frame encryption = 0b0, current frame not encrypted
                                                    // [3:2]: fixed = 0b01
                                                    // [1]:   HDMI/DVI mode select = 0b1, HDMI mode
                                                    // [0]:   fixed = 0b0
            6: write_i2c(CHIP_ADDR, 16'h_BA_60); // [7:5]: clock delay, 0b011 no delay
                                                    // [4]:   hdcp eprom, 0b0 external
                                                    // [3]:   fixed, 0b0
                                                    // [2]:   display aksv, 0b0 don't show
                                                    // [1]:   Ri two point check, 0b0 hdcp Ri standard
            7: write_i2c(CHIP_ADDR, 16'h_0A_00); // [7]:   CTS selet = 0b0, automatic
                                                    // [6:4]: audio select = 0b000, I2S
                                                    // [3:2]: audio mode = 0b00, default (HBR not used)
                                                    // [1:0]: MCLK Ratio = 0b00, 128xfs
            8: write_i2c(CHIP_ADDR, 16'h_01_00); // [3:0] \
            9: write_i2c(CHIP_ADDR, 16'h_02_18); // [7:0]  |--> [19:0]: audio clock regeneration N value, 44.1kHz@automatic CTS = 0x1880 (6272)
            10: write_i2c(CHIP_ADDR, 16'h_03_80); // [7:0] /
            11: write_i2c(CHIP_ADDR, 16'h_0B_0E); // [7]:   SPDIF enable = 0b0, disable
                                                    // [6]:   audio clock polarity = 0b0, rising edge
                                                    // [5]:   MCLK enable = 0b0, MCLK internally generated
                                                    // [4:1]: fixed = 0b0111
            12: write_i2c(CHIP_ADDR, 16'h_0C_05); // [7]:   audio sampling frequency select = 0b0, use sampling frequency from I2S stream
                                                    // [6]:   channel status override = 0b0, use channel status bits from I2S stream
                                                    // [5]:   I2S3 enable = 0b0, disabled
                                                    // [4]:   I2S2 enable = 0b0, disabled
                                                    // [3]:   I2S1 enable = 0b0, disabled
                                                    // [2]:   I2S0 enable = 0b1, enabled
                                                    // [1:0]: I2S format = 0b01, right justified mode
            13: write_i2c(CHIP_ADDR, 16'h_0D_10); // [4:0]: I2S bit width = 0b10000, 16bit
            14: write_i2c(CHIP_ADDR, { 8'h_3B, hdmiVideoConfig.adv_reg_3b });
                                                // [7]:   fixed = 0b1
                                                // [6:5]: PR Mode = 0b10, manual mode
                                                // [4:3]: PR PLL Manual = 0b01, x2
                                                // [2:1]: PR Value Manual = 0b00, x1 to rx
                                                // [0]:   fixed = 0b0
            15: write_i2c(CHIP_ADDR, { 8'h_3C, hdmiVideoConfig.adv_reg_3c });
                                                // [5:0]: VIC Manual = 010000, VIC#16: 1080p-60, 16:9
                                                // 000000, VIC#0: VIC Unavailable
            default: begin
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_bootstrap;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_41_10); // [6]:   power down = 0b0, all circuits powered up
                                                 // [5]:   fixed = 0b0
                                                 // [4]:   reserved = 0b1
                                                 // [3:2]: fixed = 0b00
                                                 // [1]:   sync adjustment enable = 0b0, disabled
                                                 // [0]:   fixed = 0b0
            1: write_i2c(CHIP_ADDR, 16'h_98_03); // Fixed register
            2: write_i2c(CHIP_ADDR, 16'h_9A_E0); // Fixed register
            3: write_i2c(CHIP_ADDR, 16'h_9C_30); // Fixed register
            4: write_i2c(CHIP_ADDR, 16'h_9D_01); // Fixed register
            5: write_i2c(CHIP_ADDR, 16'h_A2_A4); // Fixed register
            6: write_i2c(CHIP_ADDR, 16'h_A3_A4); // Fixed register
            7: write_i2c(CHIP_ADDR, 16'h_E0_D0); // Fixed register
            8: write_i2c(CHIP_ADDR, 16'h_F9_00); // Fixed register	
            9: write_i2c(CHIP_ADDR, 16'h_94_C0); // [7]:   HPD interrupt = 0b1, enabled
                                                 // [6]:   monitor sense interrupt = 0b1, enabled
                                                 // [5]:   vsync interrupt = 0b0, disabled
                                                 // [4]:   audio fifo full interrupt = 0b0, disabled
                                                 // [3]:   fixed = 0b0
                                                 // [2]:   EDID ready interrupt = 0b0, disabled
                                                 // [1]:   HDCP authenticated interrupt = 0b0, disabled
                                                 // [0]:   fixed = 0b0
            10: write_i2c(CHIP_ADDR, 16'h_96_C0); // [7]:   HPD interrupt = 0b1, interrupt detected
                                                 // [6]:   monitor sense interrupt = 0b1, interrupt detected
                                                 // [5]:   vsync interrupt = 0b0, no interrupt detected
                                                 // [4]:   audio fifo full interrupt = 0b0, no interrupt detected
                                                 // [3]:   fixed = 0b0
                                                 // [2]:   EDID ready interrupt = 0b0, no interrupt detected
                                                 // [1]:   HDCP authenticated interrupt = 0b0, no interrupt detected
                                                 // [0]:   fixed = 0b0
                                                 // -> clears interrupt state
            default: begin
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_link_powerdown;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_41_10); // [6]:   power down = 0b0, all circuits powered up
                                                 // [5]:   fixed = 0b0
                                                 // [4]:   reserved = 0b1
                                                 // [3:2]: fixed = 0b00
                                                 // [1]:   sync adjustment enable = 0b0, disabled
                                                 // [0]:   fixed = 0b0
            1: write_i2c(CHIP_ADDR, 16'h_D6_10);
            2: write_i2c(CHIP_ADDR, 16'h_A1_3C);
            default: begin
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_link_powerup;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_A1_00);
            1: write_i2c(CHIP_ADDR, 16'h_D6_00);
            default: begin
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_pllcheck;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: read_i2c(CHIP_ADDR, 8'h_9E);
            1: begin
                if (i2c_data[4]) begin
                    cmd_counter <= next_cmd;
                    subcmd_counter <= scs_start;
                end else begin // loop until pll locks
                    debugData.pll_errors <= debugData.pll_errors + 1'b1;
                    cmd_counter <= cs_pllcheck;
                    subcmd_counter <= scs_start;
                end
            end
        endcase
    end
endtask

task adv7513_ctscheck;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: read_i2c(CHIP_ADDR, 8'h_04);
            1: begin
                do_cts(debugData.cts1_status, debugData.max_cts1_status, debugData.max_cts1_status, debugData.summary_cts1_status, debugData.summary_cts1_status);
            end

            2: read_i2c(CHIP_ADDR, 8'h_05);
            3: begin
                do_cts(debugData.cts2_status, debugData.max_cts2_status, debugData.max_cts2_status, debugData.summary_cts2_status, debugData.summary_cts2_status);
            end

            4: read_i2c(CHIP_ADDR, 8'h_06);
            5: begin
                do_cts(debugData.cts3_status, debugData.max_cts3_status, debugData.max_cts3_status, debugData.summary_cts3_status, debugData.summary_cts3_status);
            end

            6: read_i2c(CHIP_ADDR, 8'h_06);
            7: begin
                do_cts(debugData.cts3_status, debugData.max_cts3_status, debugData.max_cts3_status, debugData.summary_cts3_status, debugData.summary_cts3_status);
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_debug;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: read_i2c(CHIP_ADDR, 8'h_00);
            1: begin
                debugData.chip_revision <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
            end

            2: read_i2c(CHIP_ADDR, 8'h_F5);
            3: begin
                debugData.id_check_high <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
            end

            4: read_i2c(CHIP_ADDR, 8'h_F6);
            5: begin
                debugData.id_check_low <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
            end

            6: read_i2c(CHIP_ADDR, 8'h_9E);
            7: begin
                debugData.pll_status <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
                if (!i2c_data[4]) begin
                    debugData.pll_errors <= debugData.pll_errors + 1'b1;
                end
            end

            8: read_i2c(CHIP_ADDR, 8'h_3E);
            9: begin
                debugData.vic_detected <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
            end

            10: read_i2c(CHIP_ADDR, 8'h_3D);
            11: begin
                debugData.vic_to_rx <= i2c_data;
                subcmd_counter <= subcmd_counter + 1'b1;
            end

            12: read_i2c(CHIP_ADDR, 8'h_42);
            13: begin
                debugData.misc_data <= i2c_data;
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

endmodule
