module ADV7513(
	input clk,
	input reset,
	input hdmi_int,
	
	inout sda,
	inout scl,
	output reg ready
);

wire i2c_ena;
wire [6:0] i2c_addr;
wire i2c_rw;
wire [7:0] i2c_data_wr;
reg i2c_busy;
reg [7:0] i2c_data_rd;
reg i2c_ack_error;

defparam i2c_master.input_clk = 25_200_000;
//defparam i2c_master.input_clk = 108_000_000;
//defparam i2c_master.input_clk = 148_500_000;
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
	.scl		  (scl)
);	

reg [6:0] i2c_chip_addr;
reg [7:0] i2c_reg_addr;
reg [7:0] i2c_value;
wire i2c_enable;
reg i2c_enable_reg;

wire [7:0] i2c_data;
wire i2c_done;

I2C_write I2C_write(
	.clk 			 (clk),
	.reset		 (1'b1),

	.chip_addr	 (i2c_chip_addr),
	.reg_addr	 (i2c_reg_addr),
	.value		 (i2c_value),
	.enable		 (i2c_enable),
	.done			 (i2c_done),

	.i2c_busy	 (i2c_busy),
	.i2c_ena		 (i2c_ena),
	.i2c_addr	 (i2c_addr),
	.i2c_rw		 (i2c_rw),
	.i2c_data_wr (i2c_data_wr)
);


(* syn_encoding = "safe" *)
reg [1:0] state;
reg [5:0] cmd_counter;

localparam CHIP_ADDR = 7'h39;

localparam s_start  = 0,
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
		i2c_enable_reg <= 1'b0;
		ready <= 0;
	end else begin
		case (state)
			
			s_start: begin
				if (i2c_done) begin
					
					case (cmd_counter)
					
						 0: write_i2c(CHIP_ADDR, 16'h_41_10); // [6]:   power down = 0b0, all circuits powered up
																		  // [5]:	fixed = 0b0
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
						11: write_i2c(CHIP_ADDR, 16'h_17_00); // [7]:   fixed = 0b0
																		  // [6]:   vsync polarity = 0b0, sync polarity pass through (sync adjust is off in 0x41)
																		  // [5]:   hsync polarity = 0b0, sync polarity pass through 
																		  // [4:3]: reserved = 0b00
																		  // [2]:   4:2:2 to 4:4:4 interpolation style = 0b0, use zero order interpolation
																		  // [1]:   input video aspect ratio = 0b0, 4:3 for VGA
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
						15: write_i2c(CHIP_ADDR, 16'h_0A_00); // [7]:   CTS automatic = 0b0
																		  // [6:4]: I2S = 0b000
																		  // [3:2]: default = 0b00
																		  // [1:0]: MCLK Ratio 128fs = 0b00
						16: write_i2c(CHIP_ADDR, 16'h_01_00); // audio clock regeneratrion N value, 44.1kHz@automatic CTS
						17: write_i2c(CHIP_ADDR, 16'h_02_18); // recommended N when automatically
						18: write_i2c(CHIP_ADDR, 16'h_03_80); // generating CTS is 0x1880
						
						19: write_i2c(CHIP_ADDR, 16'h_0B_0E); // disable SPDIF
						20: write_i2c(CHIP_ADDR, 16'h_0C_05); // use sample freq from stream, enable I2S0, right justified mode
						21: write_i2c(CHIP_ADDR, 16'h_0D_10); // set I2S Bit Width to 16bit
						//22: write_i2c(CHIP_ADDR, 16'h_56_18); // [7:6]: Colorimetry: nodata
																		  // [5:4]: Picture Aspect Ratio: 4:3
																		  // [3:0]: Active Format Aspect Ratio, Same as Aspect Ratio
						22: write_i2c(CHIP_ADDR, 16'h_94_C0); // Interrupt Enable: hot plug detect, monitor sense
						23: write_i2c(CHIP_ADDR, 16'h_96_C0); // clear interrupt
					
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
				i2c_enable_reg <= 1'b0;
				
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
		i2c_chip_addr  <= t_chip_addr;
		i2c_reg_addr   <= t_data[15:8];
		i2c_value      <= t_data[7:0];
		i2c_enable_reg <= 1'b1;
		state 			<= s_wait;
	end
endtask


assign i2c_enable = i2c_enable_reg;


endmodule
