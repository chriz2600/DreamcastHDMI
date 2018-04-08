`include "config.inc"

module ADV7513(
    input clk,
    input reset,
    input hdmi_int,
    input VSYNC,

    inout sda,
    inout scl,
`ifdef DEBUG
    output reg text_wren,
    output reg [9:0] text_wraddr,
    output reg [7:0] text_wrdata,
`endif
    output reg ready
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
    .i2c_ack_error (i2c_ack_error)
);

(* syn_encoding = "safe" *)
reg [1:0] state;
reg [5:0] cmd_counter;
reg [7:0] pll_errors = 0;

`ifdef DEBUG
reg VSYNC_reg = 0;
reg trigger = 0;
reg [7:0] test = 0;

reg [9:0] frame_counter = 0;
reg [7:0] counter = 0;
reg [7:0] print_field = 0;

reg [7:0] pll_status = 0;
reg [7:0] id_check_high = 0;
reg [7:0] id_check_low = 0;
reg [7:0] chip_revision = 0;
reg [7:0] vic_detected = 0;
reg [7:0] vic_to_rx = 0;
reg [7:0] misc_data = 0;

reg [7:0] cts1_status = 0;
reg [7:0] cts2_status = 0;
reg [7:0] cts3_status = 0;

reg [7:0] prev_cts1_status = 0;
reg [7:0] prev_cts2_status = 0;
reg [7:0] prev_cts3_status = 0;

reg [7:0] summary_cts1_status = 0;
reg [7:0] summary_cts2_status = 0;
reg [7:0] summary_cts3_status = 0;
reg [7:0] summary_summary_cts3_status = 0;
`endif 

localparam CHIP_ADDR = 7'h39;

localparam  s_start  = 0,
            s_wait   = 1,
            s_wait_2 = 2,
            s_idle   = 3;

localparam INIT_START    = 6'd0;
localparam PLL_CHECK_1   = 6'd32;
`ifdef DEBUG
localparam CHIP_REVISION = 6'd42;
localparam ID_CHECK_H    = 6'd44;
localparam ID_CHECK_L    = 6'd46;
localparam PLL_CHECK_2   = 6'd48;
localparam CTS_CHECK_1   = 6'd50;
localparam CTS_CHECK_2   = 6'd52;
localparam CTS_CHECK_3   = 6'd54;
localparam VIC_CHECK_1   = 6'd56;
localparam VIC_CHECK_2   = 6'd58;
localparam MISC_CHECK    = 6'd60;
`endif
localparam GOTO_READY    = 6'b111111;

initial begin
    ready <= 0;
end

always @ (posedge clk) begin

    if (~reset) begin
        state <= s_start;
        cmd_counter <= 0;
        i2c_enable <= 1'b0;
        ready <= 0;
    end else begin
`ifdef DEBUG
        VSYNC_reg <= VSYNC;
`endif
        case (state)
            
            s_start: begin
                if (i2c_done) begin
                    
                    case (cmd_counter)
                    
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
                         9: write_i2c(CHIP_ADDR, 16'h_15_00); // [7:4]: I2S Sampling Frequency = 0b0000, 44.1kHz
                                                              // [3:0]: Video Input ID = 0b0000, 24 bit RGB 4:4:4 (separate syncs)
                        10: write_i2c(CHIP_ADDR, 16'h_16_30); // [7]:   output format = 0b0, 4:4:4
                                                              // [6]:   reserved = 0b0
                                                              // [5:4]: color depth = 0b11, 8bit
                                                              // [3:2]: input style = 0b0, not valid
                                                              // [1]:   ddr input edge = 0b0, falling edge
                                                              // [0]:   output colorspace for blackimage = 0b0, RGB
                        11: write_i2c(CHIP_ADDR, 16'h_17_00
                                                | `ASPECT_R); // [7]:   fixed = 0b0
                                                              // [6]:   vsync polarity = 0b0, sync polarity pass through (sync adjust is off in 0x41)
                                                              // [5]:   hsync polarity = 0b0, sync polarity pass through 
                                                              // [4:3]: reserved = 0b00
                                                              // [2]:   4:2:2 to 4:4:4 interpolation style = 0b0, use zero order interpolation
                                                              // [1]:   input video aspect ratio = 0b0, 4:3; 0b10 for 16:9
                                                              // [0]:   DE generator = 0b0, disabled
                        12: write_i2c(CHIP_ADDR, 16'h_18_46); // [7]:   CSC enable = 0b0, disabled
                                                              // [6:5]: default = 0b10
                                                              // [4:0]: default = 0b00110
                        13: write_i2c(CHIP_ADDR, 16'h_AF_06); // [7]:   HDCP enable = 0b0, disabled
                                                              // [6:5]: fixed = 0b00
                                                              // [4]:   frame encryption = 0b0, current frame not encrypted
                                                              // [3:2]: fixed = 0b01
                                                              // [1]:   HDMI/DVI mode select = 0b1, HDMI mode
                                                              // [0]:   fixed = 0b0
                        14: write_i2c(CHIP_ADDR, 16'h_BA_60); // [7:5]: clock delay, 0b011 no delay
                                                              // [4]:   hdcp eprom, 0b0 external
                                                              // [3]:   fixed, 0b0
                                                              // [2]:   display aksv, 0b0 don't show
                                                              // [1]:   Ri two point check, 0b0 hdcp Ri standard
                        15: write_i2c(CHIP_ADDR, 16'h_0A_00); // [7]:   CTS selet = 0b0, automatic
                                                              // [6:4]: audio select = 0b000, I2S
                                                              // [3:2]: audio mode = 0b00, default (HBR not used)
                                                              // [1:0]: MCLK Ratio = 0b00, 128xfs
                        16: write_i2c(CHIP_ADDR, 16'h_01_00); // [3:0] \
                        17: write_i2c(CHIP_ADDR, 16'h_02_18); // [7:0]  |--> [19:0]: audio clock regeneration N value, 44.1kHz@automatic CTS = 0x1880 (6272)
                        18: write_i2c(CHIP_ADDR, 16'h_03_80); // [7:0] /
                        19: write_i2c(CHIP_ADDR, 16'h_0B_0E); // [7]:   SPDIF enable = 0b0, disable
                                                              // [6]:   audio clock polarity = 0b0, rising edge
                                                              // [5]:   MCLK enable = 0b0, MCLK internally generated
                                                              // [4:1]: fixed = 0b0111
                        20: write_i2c(CHIP_ADDR, 16'h_0C_05); // [7]:   audio sampling frequency select = 0b0, use sampling frequency from I2S stream
                                                              // [6]:   channel status override = 0b0, use channel status bits from I2S stream
                                                              // [5]:   I2S3 enable = 0b0, disabled
                                                              // [4]:   I2S2 enable = 0b0, disabled
                                                              // [3]:   I2S1 enable = 0b0, disabled
                                                              // [2]:   I2S0 enable = 0b1, enabled
                                                              // [1:0]: I2S format = 0b01, right justified mode
                        21: write_i2c(CHIP_ADDR, 16'h_0D_10); // [4:0]: I2S bit width = 0b10000, 16bit
                        //22: write_i2c(CHIP_ADDR, 16'h_56_18); // [7:6]: Colorimetry = 0b00, no data
                                                              // [5:4]: Picture Aspect Ratio (AVI Info frame) = 0b01, 4:3
                                                              // [3:0]: Active Format Aspect Ratio = 0b1000, Same as Aspect Ratio
                        22: write_i2c(CHIP_ADDR, 16'h_94_C0); // [7]:   HPD interrupt = 0b1, enabled
                                                              // [6]:   monitor sense interrupt = 0b1, enabled
                                                              // [5]:   vsync interrupt = 0b0, disabled
                                                              // [4]:   audio fifo full interrupt = 0b0, disabled
                                                              // [3]:   fixed = 0b0
                                                              // [2]:   EDID ready interrupt = 0b0, disabled
                                                              // [1]:   HDCP authenticated interrupt = 0b0, disabled
                                                              // [0]:   fixed = 0b0
                        23: write_i2c(CHIP_ADDR, 16'h_96_C0); // [7]:   HPD interrupt = 0b1, interrupt detected
                                                              // [6]:   monitor sense interrupt = 0b1, interrupt detected
                                                              // [5]:   vsync interrupt = 0b0, no interrupt detected
                                                              // [4]:   audio fifo full interrupt = 0b0, no interrupt detected
                                                              // [3]:   fixed = 0b0
                                                              // [2]:   EDID ready interrupt = 0b0, no interrupt detected
                                                              // [1]:   HDCP authenticated interrupt = 0b0, no interrupt detected
                                                              // [0]:   fixed = 0b0
                                                              // -> clears interrupt state
`ifdef PIXEL_REPETITION
                        24: write_i2c(CHIP_ADDR, 16'h_3B_C8); // [7]:   fixed = 0b1
                                                              // [6:5]: PR Mode = 0b10, manual mode
                                                              // [4:3]: PR PLL Manual = 0b01, x2
                                                              // [2:1]: PR Value Manual = 0b00, x1 to rx
                                                              // [0]:   fixed = 0b0
                        25: write_i2c(CHIP_ADDR, 16'h_3C_00
                                              | `VIC_MANUAL); // [5:0]: VIC Manual = 010000, VIC#16: 1080p-60, 16:9
                                                              //                     000000, VIC#0: VIC Unavailable
                        26: cmd_counter <= PLL_CHECK_1;
`else
                        24: cmd_counter <= PLL_CHECK_1;
`endif
                        PLL_CHECK_1: read_i2c(CHIP_ADDR, 8'h_9E);
                        (PLL_CHECK_1+1): begin
                            if (i2c_data[4]) begin
                                cmd_counter <= GOTO_READY;
                            end else begin
                                pll_errors <= pll_errors + 1'b1;
                                cmd_counter <= INIT_START;
                            end
                        end

`ifdef DEBUG
                        CHIP_REVISION: read_i2c(CHIP_ADDR, 8'h_00);
                        (CHIP_REVISION+1): begin
                            chip_revision <= i2c_data;
                            cmd_counter <= ID_CHECK_H;
                        end

                        ID_CHECK_H: read_i2c(CHIP_ADDR, 8'h_F5);
                        (ID_CHECK_H+1): begin
                            id_check_high <= i2c_data;
                            cmd_counter <= ID_CHECK_L;
                        end

                        ID_CHECK_L: read_i2c(CHIP_ADDR, 8'h_F6);
                        (ID_CHECK_L+1): begin
                            id_check_low <= i2c_data;
                            cmd_counter <= PLL_CHECK_2;
                        end

                        PLL_CHECK_2: read_i2c(CHIP_ADDR, 8'h_9E);
                        (PLL_CHECK_2+1): begin
                            pll_status <= i2c_data;
                            cmd_counter <= CTS_CHECK_1;
                             if (!i2c_data[4]) begin
                                pll_errors <= pll_errors + 1'b1;
                             end
                        end

                        CTS_CHECK_1: read_i2c(CHIP_ADDR, 8'h_04);
                        (CTS_CHECK_1+1): begin
                            prev_cts1_status <= cts1_status;
                            cts1_status <= i2c_data;
                            cmd_counter <= CTS_CHECK_2;
                            summary_cts1_status <= summary_cts1_status | (prev_cts1_status ^ cts1_status);
                        end

                        CTS_CHECK_2: read_i2c(CHIP_ADDR, 8'h_05);
                        (CTS_CHECK_2+1): begin
                            prev_cts2_status <= cts2_status;
                            cts2_status <= i2c_data;
                            cmd_counter <= CTS_CHECK_3;
                            summary_cts2_status <= summary_cts2_status | (prev_cts2_status ^ cts2_status);
                        end

                        CTS_CHECK_3: read_i2c(CHIP_ADDR, 8'h_06);
                        (CTS_CHECK_3+1): begin
                            prev_cts3_status <= cts3_status;
                            cts3_status <= i2c_data;
                            cmd_counter <= VIC_CHECK_1;
                            summary_cts3_status <= summary_cts3_status | (prev_cts3_status ^ cts3_status);
                        end

                        VIC_CHECK_1: read_i2c(CHIP_ADDR, 8'h_3E);
                        (VIC_CHECK_1+1): begin
                            vic_detected <= i2c_data;
                            cmd_counter <= VIC_CHECK_2;
                        end

                        VIC_CHECK_2: read_i2c(CHIP_ADDR, 8'h_3D);
                        (VIC_CHECK_2+1): begin
                            vic_to_rx <= i2c_data;
                            cmd_counter <= MISC_CHECK;
                        end

                        MISC_CHECK: read_i2c(CHIP_ADDR, 8'h_42);
                        (MISC_CHECK+1): begin
                            misc_data <= i2c_data;
                            cmd_counter <= GOTO_READY;
                        end
`endif
                        default: begin
                            cmd_counter <= INIT_START;
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
                        cmd_counter <= cmd_counter + 1'b1;
                    end 
                    state <= s_start;
                end
            end

            s_idle: begin
                if (~hdmi_int) begin
                    state <= s_start;
                    ready <= 0;
`ifdef DEBUG
                    trigger <= 0;
                end else if (~VSYNC_reg && VSYNC) begin
                    cmd_counter <= CHIP_REVISION;
                    state <= s_start;
                    trigger <= 0;
                    frame_counter <= frame_counter + 1'b1;
                end else if (VSYNC_reg && ~VSYNC) begin
                    trigger <= 1;
                    print_field <= 1;
                    counter <= 0;
                end

                if (frame_counter == 1023) begin
                    summary_cts1_status <= 0;
                    summary_cts2_status <= 0;
                    summary_cts3_status <= 0;
                    summary_summary_cts3_status <= summary_cts3_status;
                    frame_counter <= 0;
                    test <= test + 1'b1;
                end

                if (trigger && print_field > 0) begin
                    text_wren <= 1;
                    case (print_field)
                        1: print_status(272 -  40, chip_revision, 7, 0);
                        2: print_status(272 +   0, id_check_high, 7, 0);
                        3: print_status(272 +  40, id_check_low, 7, 0);
                        4: print_status(272 +  80 + 7, pll_status, 4, 4);
                        5: print_status(272 + 120, pll_errors, 7, 0);
                        6: print_status(272 + 200 + 4, cts1_status, 3, 0);
                        7: print_status(272 + 240 + 4, summary_cts1_status, 3, 0);
                        8: print_status(272 + 280, cts2_status, 7, 0);
                        9: print_status(272 + 320, summary_cts2_status, 7, 0);
                        10: print_status(272 + 360, cts3_status, 7, 0);
                        11: print_status(272 + 400, summary_cts3_status, 7, 0);
                        12: print_status(272 + 440, summary_summary_cts3_status, 7, 0);
                        13: print_status(272 + 520 - 7, vic_detected[7:2], 5, 0);
                        14: print_status(272 + 520 + 2, vic_to_rx[5:0], 5, 0);
                        15: print_status(272 + 560 - 1, misc_data, 6, 6);
                        16: print_status(272 + 560 + 3, misc_data, 5, 5);
                        17: print_status(272 + 560 + 7, misc_data, 3, 3);
                        18: print_status(272 + 680, test, 7, 0);

                        default: begin
                            print_field <= 0;
                        end
                    endcase
                end else begin
                    text_wren <= 0;
`endif
                end
            end
            
        endcase
    end
end

`ifdef DEBUG
task print_status;
    input [9:0] addr;
    input [7:0] data;
    input [4:0] high;
    input [4:0] low;

    begin
        if (counter < (high - low + 1)) begin
            text_wraddr <= addr + counter;
            text_wrdata <= data[high - counter] ? "1" : "0";
            counter <= counter + 1'b1;
        end else begin
            print_field <= print_field + 1'b1;
            counter <= 0;
        end
    end
endtask
`endif

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

endmodule
