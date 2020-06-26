`include "config.inc"

module ADV7513(
    input clk,
    input reset,
    input hdmi_int,
    input output_ready,

    inout sda,
    inout scl,
    output reg ready,
    output reg hdmi_int_reg,
    output reg hpd_detected,
    output reg [31:0] pll_adv_lockloss_count,
    output reg [31:0] hpd_low_count,
    output reg [31:0] monitor_sense_low_count,

    input [7:0] clock_data,
    input [1:0] colorspace,
    input ADV7513Config adv7513Config,

    output i2c_working
);

reg [6:0] i2c_chip_addr;
reg [7:0] i2c_reg_addr;
reg [7:0] i2c_value;
reg i2c_enable;
reg i2c_is_read;
reg i2c_working_reg;

assign i2c_working = i2c_working_reg;

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

    .divider       (32'h_C8_96_64_32)
);

(* syn_encoding = "safe" *) 
reg [1:0] state;
reg [2:0] cmd_counter;
reg [5:0] subcmd_counter;

localparam CHIP_ADDR = 7'h39;
localparam  s_start  = 0,
            s_wait   = 1,
            s_wait_2 = 2,
            s_idle   = 3;
localparam  cs_pwrdown  = 3'd0,
            cs_init     = 3'd1,
            cs_init2    = 3'd2,
            cs_pllcheck = 3'd3,
            cs_hpdcheck = 3'd4,
            cs_ready    = 3'd5;
localparam  scs_start = 6'd0;

reg hdmi_int_prev = 1;
reg prev_pll_adv_lockloss = 0;
reg prev_hpd_state = 0;
reg prev_monitor_sense_state = 0;

initial begin
    ready = 0;
    i2c_working_reg = 0;
    pll_adv_lockloss_count = 0;
    hpd_low_count = 0;
    monitor_sense_low_count = 0;
end

reg [32:0] counter = 0;

always @ (posedge clk) begin

    if (hdmi_int_prev && ~hdmi_int) begin
        hdmi_int_reg = 1'b1;
    end
    hdmi_int_prev <= hdmi_int;

    if (~reset) begin
        state <= s_start;
        cmd_counter <= cs_pwrdown;
        subcmd_counter <= scs_start;
        i2c_enable <= 1'b0;
    end else begin
        case (state)
            s_start: begin
                if (i2c_done) begin
                    case (cmd_counter)
                        cs_pwrdown: begin
                            ready <= 1'b0;
                            adv7513_powerdown(cs_init);
                        end
                        cs_init: adv7513_monitor_hpd(cs_init2, cs_pwrdown);
                        cs_init2: adv7513_init(cs_pllcheck);
                        cs_pllcheck: adv7513_pllcheck(cs_ready, cs_pllcheck);

                        cs_hpdcheck: adv7513_monitor_hpd(cs_ready, cs_pwrdown);

                        default: begin
                            cmd_counter <= cs_init;
                            subcmd_counter <= scs_start;
                            state <= s_idle;
                            ready <= 1'b1;
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
                if (hdmi_int_reg) begin
                    state <= s_start;
                    cmd_counter <= cs_pwrdown;
                    subcmd_counter <= scs_start;
                    hdmi_int_reg = 1'b0;
                end else if (counter == 32'd_16_000_000) begin
                    state <= s_start;
                    cmd_counter <= cs_hpdcheck;
                    subcmd_counter <= scs_start;
                    counter <= 1'b0;
                end else begin
                    counter <= counter + 1'b1;
                end
            end
            
        endcase
    end
end

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
task adv7513_monitor_hpd;
    input [2:0] success_cmd;
    input [2:0] failure_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_96_F6); // -> clears interrupt state, mark all interrupts as detected
            // check pll status
            1: read_i2c(CHIP_ADDR, 8'h_9E);
            2: begin
                prev_pll_adv_lockloss <= ~i2c_data[4];
                if (~prev_pll_adv_lockloss && ~i2c_data[4]) begin
                    pll_adv_lockloss_count <= pll_adv_lockloss_count + 1'b1;
                end
                subcmd_counter <= subcmd_counter + 1'b1;
            end
            // monitor HPD State/Monitor Sense State
            3: read_i2c(CHIP_ADDR, 8'h_42);
            4: begin
                // debug
                prev_hpd_state <= i2c_data[6];
                prev_monitor_sense_state <= i2c_data[5];
                if (prev_hpd_state && ~i2c_data[6]) begin
                    hpd_low_count <= hpd_low_count + 1'b1;
                end
                if (prev_monitor_sense_state && ~i2c_data[5]) begin
                    monitor_sense_low_count <= monitor_sense_low_count + 1'b1;
                end

                if (i2c_data[6] && i2c_data[5]) begin
                    cmd_counter <= success_cmd;
                    subcmd_counter <= scs_start;
                    hpd_detected <= 1'b1;
                end else begin
                    cmd_counter <= failure_cmd;
                    subcmd_counter <= scs_start;
                    hpd_detected <= 1'b0;
                end
            end
        endcase
    end
endtask

task adv7513_init;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_41_10); // [6]:   power down = 0b0, all circuits powered up
                                                 // [5]:   fixed = 0b0
                                                 // [4]:   reserved = 0b1
                                                 // [3:2]: fixed = 0b00
                                                 // [1]:   sync adjustment enable = 0b0, disabled
                                                 // [0]:   fixed = 0b0
            1: write_i2c(CHIP_ADDR, 16'h_49_A8); // ADI recommended write
            2: write_i2c(CHIP_ADDR, 16'h_4C_00); // ADI recommended write
            3: write_i2c(CHIP_ADDR, 16'h_15_00); // [7:4]: I2S Sampling Frequency = 0b0000, 44.1kHz
                                                // [3:0]: Video Input ID = 0b0000, 24 bit RGB 4:4:4 (separate syncs)
            4: write_i2c(CHIP_ADDR, 16'h_16_30 | `OUTPUT_FMT);
                                                // [7]:   output format = 0b0, 4:4:4, (4:2:2, if OUTPUT_4_2_2 is set)
                                                // [6]:   reserved = 0b0
                                                // [5:4]: color depth = 0b11, 8bit
                                                // [3:2]: input style = 0b0, not valid
                                                // [1]:   ddr input edge = 0b0, falling edge
                                                // [0]:   output colorspace for blackimage = 0b0, RGB (YCbCr, if OUTPUT_4_2_2 is set)
            5: write_i2c(CHIP_ADDR, { 8'h_17, adv7513Config.adv_reg_17 });
                                                // [7]:   fixed = 0b0
                                                // [6]:   vsync polarity = 0b0, sync polarity pass through (sync adjust is off in 0x41)
                                                // [5]:   hsync polarity = 0b0, sync polarity pass through
                                                // [4:3]: reserved = 0b00
                                                // [2]:   4:2:2 to 4:4:4 interpolation style = 0b0, use zero order interpolation
                                                // [1]:   input video aspect ratio = 0b0, 4:3; 0b10 for 16:9
                                                // [0]:   DE generator = 0b0, disabled
            6: write_i2c(CHIP_ADDR, 16'h_18_46); // [7]:   CSC enable = 0b0, disabled
                                                // [6:5]: default = 0b10
                                                // [4:0]: default = 0b00110
            7: write_i2c(CHIP_ADDR, 16'h_40_80); // ???
            8: write_i2c(CHIP_ADDR, 16'h_98_03); // ADI recommended write
            9: write_i2c(CHIP_ADDR, 16'h_99_02); // ADI recommended write
            10: write_i2c(CHIP_ADDR, 16'h_9A_E0); // ADI recommended write
            11: write_i2c(CHIP_ADDR, 16'h_9C_30); // ADI recommended write
            12: write_i2c(CHIP_ADDR, 16'h_9D_61); // ADI recommended write
            13: write_i2c(CHIP_ADDR, 16'h_A2_A4); // ADI recommended write
            14: write_i2c(CHIP_ADDR, 16'h_A3_A4); // ADI recommended write
            15: write_i2c(CHIP_ADDR, 16'h_A5_04); // ADI recommended write
            16: write_i2c(CHIP_ADDR, 16'h_AB_40); // ADI recommended write
            17: write_i2c(CHIP_ADDR, 16'h_AF_16); // [7]:   HDCP enable = 0b0, disabled
                                                // [6:5]: fixed = 0b00
                                                // [4]:   frame encryption = 0b0, current frame not encrypted
                                                // [3:2]: fixed = 0b01
                                                // [1]:   HDMI/DVI mode select = 0b1, HDMI mode
                                                // [0]:   fixed = 0b0
            18: write_i2c(CHIP_ADDR, 16'h_DE_10); // ADI recommended write
            19: write_i2c(CHIP_ADDR, 16'h_BA_00 | clock_data); // [7:5]: clock delay, 0b011 no delay
                                                // [4]:   hdcp eprom, 0b1 internal
                                                // [3]:   fixed, 0b0
                                                // [2]:   display aksv, 0b0 don't show
                                                // [1]:   Ri two point check, 0b0 hdcp Ri standard
            20: write_i2c(CHIP_ADDR, 16'h_D1_FF); // ADI recommended write
            21: write_i2c(CHIP_ADDR, 16'h_E4_60); // ADI recommended write
            22: write_i2c(CHIP_ADDR, 16'h_FA_7D); // ???
            23: write_i2c(CHIP_ADDR, 16'h_E0_D0); // Fixed register
            24: write_i2c(CHIP_ADDR, 16'h_F9_00); // Fixed register
            25: write_i2c(CHIP_ADDR, { 8'h_01, adv7513Config.adv_reg_01 });
            26: write_i2c(CHIP_ADDR, { 8'h_02, adv7513Config.adv_reg_02 });
            27: write_i2c(CHIP_ADDR, { 8'h_03, adv7513Config.adv_reg_03 });
            28: write_i2c(CHIP_ADDR, 16'h_0A_00); // [7]:   CTS selet = 0b0, automatic
                                                // [6:4]: audio select = 0b000, I2S
                                                // [3:2]: audio mode = 0b00, default (HBR not used)
                                                // [1:0]: MCLK Ratio = 0b00, 128xfs
            29: write_i2c(CHIP_ADDR, 16'h_0C_05); // [7]:   audio sampling frequency select = 0b0, use sampling frequency from I2S stream
                                                // [6]:   channel status override = 0b0, use channel status bits from I2S stream
                                                // [5]:   I2S3 enable = 0b0, disabled
                                                // [4]:   I2S2 enable = 0b0, disabled
                                                // [3]:   I2S1 enable = 0b0, disabled
                                                // [2]:   I2S0 enable = 0b1, enabled
                                                // [1:0]: I2S format = 0b01, right justified mode
            30: write_i2c(CHIP_ADDR, 16'h_0D_10); // [4:0]: I2S bit width = 0b10000, 16bit
            31: write_i2c(CHIP_ADDR, { 8'h_3B, adv7513Config.adv_reg_3b });
                                                // [7]:   fixed = 0b1
                                                // [6:5]: PR Mode = 0b10, manual mode
                                                // [4:3]: PR PLL Manual = 0b01, x2
                                                // [2:1]: PR Value Manual = 0b00, x1 to rx
                                                // [0]:   fixed = 0b0
            32: write_i2c(CHIP_ADDR, { 8'h_3C, adv7513Config.adv_reg_3c });
                                                // [5:0]: VIC Manual = 010000, VIC#16: 1080p-60, 16:9
                                                // 000000, VIC#0: VIC Unavailable
            33: write_i2c(CHIP_ADDR, 16'h_94_C0); // [7]:   HPD interrupt = 0b1, enabled
                                                // [6]:   monitor sense interrupt = 0b1, enabled
                                                // [5]:   vsync interrupt = 0b0, disabled
                                                // [4]:   audio fifo full interrupt = 0b0, disabled
                                                // [3]:   fixed = 0b0
                                                // [2]:   EDID ready interrupt = 0b0, disabled
                                                // [1]:   HDCP authenticated interrupt = 0b0, disabled
                                                // [0]:   fixed = 0b0
            34: write_i2c(CHIP_ADDR, 16'h_55_10); // RGB in AVI InfoFrame
            35: write_i2c(CHIP_ADDR, { 8'h_56, adv7513Config.adv_reg_56 }); // AVI InfoFrame: Picture Aspect Ratio, Active Format Aspect Ratio
            36: begin
                if (colorspace == 2'd_1 || (colorspace == 0 && adv7513Config.fullrange)) begin
                    write_i2c(CHIP_ADDR, 16'h_57_08); // xvYCC 601, full range
                end else begin
                    write_i2c(CHIP_ADDR, 16'h_57_04); // xvYCC 601, limited range
                end
            end
            37: begin
                if (colorspace == 2'd_1 || (colorspace == 0 && adv7513Config.fullrange)) begin
                    write_i2c(CHIP_ADDR, 16'h_59_40); // YQ[1:0] full range
                end else begin
                    write_i2c(CHIP_ADDR, 16'h_59_00); // YQ[1:0] limited range
                end
            end
            38: begin
                if (colorspace == 2'd_1 || (colorspace == 0 && adv7513Config.fullrange)) begin
                    write_i2c(CHIP_ADDR, 16'h_18_0D); // disable CSC
                end else begin
                    write_i2c(CHIP_ADDR, 16'h_18_8D); // enable CSC
                end
            end
            39: write_i2c(CHIP_ADDR, 16'h_19_BC);
            40: write_i2c(CHIP_ADDR, 16'h_1A_00);
            41: write_i2c(CHIP_ADDR, 16'h_1B_00);
            42: write_i2c(CHIP_ADDR, 16'h_1C_00);
            43: write_i2c(CHIP_ADDR, 16'h_1D_00);
            44: write_i2c(CHIP_ADDR, 16'h_1E_01);
            45: write_i2c(CHIP_ADDR, 16'h_1F_00);
            46: write_i2c(CHIP_ADDR, 16'h_20_00);
            47: write_i2c(CHIP_ADDR, 16'h_21_00);
            48: write_i2c(CHIP_ADDR, 16'h_22_0D);
            49: write_i2c(CHIP_ADDR, 16'h_23_BC);
            50: write_i2c(CHIP_ADDR, 16'h_24_00);
            51: write_i2c(CHIP_ADDR, 16'h_25_00);
            52: write_i2c(CHIP_ADDR, 16'h_26_01);
            53: write_i2c(CHIP_ADDR, 16'h_27_00);
            54: write_i2c(CHIP_ADDR, 16'h_28_00);
            55: write_i2c(CHIP_ADDR, 16'h_29_00);
            56: write_i2c(CHIP_ADDR, 16'h_2A_00);
            57: write_i2c(CHIP_ADDR, 16'h_2B_00);
            58: write_i2c(CHIP_ADDR, 16'h_2C_0D);
            59: write_i2c(CHIP_ADDR, 16'h_2D_BC);
            60: write_i2c(CHIP_ADDR, 16'h_2E_01);
            61: write_i2c(CHIP_ADDR, 16'h_2F_00);
            default: begin
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_powerdown;
    input [2:0] next_cmd;

    begin
        case (subcmd_counter)
            0: write_i2c(CHIP_ADDR, 16'h_41_50);
            1: write_i2c(CHIP_ADDR, 16'h_D6_D1); // enable soft turn on
            2: write_i2c(CHIP_ADDR, 16'h_A1_3C); // power down all TMDS channels
            default: begin
                i2c_working_reg <= 1'b1;
                cmd_counter <= next_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

task adv7513_pllcheck;
    input [2:0] success_cmd;
    input [2:0] failure_cmd;

    begin
        case (subcmd_counter)
            0: begin
                if (output_ready) begin
                    // proceed to next command
                    subcmd_counter <= subcmd_counter + 1'b1;
                end else begin
                    cmd_counter <= failure_cmd;
                    subcmd_counter <= scs_start;
                end
            end
            1: write_i2c(CHIP_ADDR, 16'h_A1_00); // power up TMDS channels
            2: write_i2c(CHIP_ADDR, 16'h_D6_C0); // disable soft turn on
            default: begin
                cmd_counter <= success_cmd;
                subcmd_counter <= scs_start;
            end
        endcase
    end
endtask

endmodule
