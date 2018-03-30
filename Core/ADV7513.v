`include "config.inc"

module ADV7513(
    input clk,
    input reset,
    input hdmi_int,

    inout sda,
    inout scl,
    output reg ready,
    output [7:0] data_out
);

wire i2c_ena;
wire [6:0] i2c_addr;
wire i2c_rw;
wire [7:0] i2c_data_wr;
reg i2c_busy;
reg [7:0] i2c_data_rd;
reg i2c_ack_error;

defparam i2c_master.input_clk = `PIXEL_CLK;
defparam i2c_master.bus_clk = 20_000;
i2c_master i2c_master(
    .clk       (clk),
    .reset_n   (1'b1),
    .ena       (i2c_ena),
    .addr      (i2c_addr),
    .rw        (i2c_rw),
    .data_wr   (i2c_data_wr),
    .busy      (i2c_busy),
    .data_rd   (i2c_data_rd),
    .ack_error (i2c_ack_error),
    .sda       (sda),
    .scl       (scl)
);	

reg [6:0] i2c_chip_addr;
reg [7:0] i2c_reg_addr;
reg [7:0] i2c_value;
reg i2c_enable;
reg i2c_is_read;

wire [7:0] i2c_data;
wire i2c_done;

I2C I2C(
    .clk         (clk),
    .reset       (1'b1),

    .chip_addr   (i2c_chip_addr),
    .reg_addr    (i2c_reg_addr),
    .value       (i2c_value),
    .enable      (i2c_enable),
    .done        (i2c_done),

    .data        (i2c_data),

    .i2c_busy    (i2c_busy),
    .i2c_ena     (i2c_ena),
    .i2c_addr    (i2c_addr),
    .i2c_rw      (i2c_rw),
    .i2c_data_rd (i2c_data_rd),
    .i2c_data_wr (i2c_data_wr),

    .is_read     (i2c_is_read)
);

(* syn_encoding = "safe" *)
reg [1:0] state;
reg [5:0] cmd_counter;

localparam CHIP_ADDR = 7'h39;

localparam  s_start  = 0,
            s_wait   = 1,
            s_wait_2 = 2,
            s_idle   = 3;

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
`endif
                        default: begin
                            // todo monitor PLL locked state bef
                            cmd_counter <= 0;
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

endmodule
