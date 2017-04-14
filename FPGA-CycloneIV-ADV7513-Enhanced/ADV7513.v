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
					
						 0: write_i2c(CHIP_ADDR, 16'h4110);	// Power-up Tx
						 1: write_i2c(CHIP_ADDR, 16'h9803);	// Fixed register
						 2: write_i2c(CHIP_ADDR, 16'h9AE0);	// Fixed register
						 3: write_i2c(CHIP_ADDR, 16'h9C30);	// Fixed register
						 4: write_i2c(CHIP_ADDR, 16'h9D01);	// Fixed register
						 5: write_i2c(CHIP_ADDR, 16'hA2A4);	// Fixed register
						 6: write_i2c(CHIP_ADDR, 16'hA3A4);	// Fixed register
						 7: write_i2c(CHIP_ADDR, 16'hE0D0);	// Fixed register
						 8: write_i2c(CHIP_ADDR, 16'hF900);	// Fixed register	
						 9: write_i2c(CHIP_ADDR, 16'h1500);	// [3:0] Video Format ID = 0x00 4:4:4
																		// [7:4] I2S Sampling Frequency = 0x00 
						10: write_i2c(CHIP_ADDR, 16'h1630);	// Video Format 
						//10: write_i2c(CHIP_ADDR, 16'h1670);	// Video Format 
						11: write_i2c(CHIP_ADDR, 16'h1700);	// Video Format 
						12: write_i2c(CHIP_ADDR, 16'h1846);
						13: write_i2c(CHIP_ADDR, 16'hAF06);
						//17: write_i2c(CHIP_ADDR, 16'h0A01);
						//14: write_i2c(CHIP_ADDR, 16'h0A80);
						
						// [7]:   CTS automatic = 0
						// [6:4]: I2S = 000
						// [3:2]: default = 00
						// [1:0]: MCLK Ratio 128fs = 00
						14: write_i2c(CHIP_ADDR, 16'h0A00); 

						// Audip Clock Regeneratrion N Value, 44.1kHz@automatic CTS = 0x1880
						15: write_i2c(CHIP_ADDR, 16'h0100);
						16: write_i2c(CHIP_ADDR, 16'h0218);
						17: write_i2c(CHIP_ADDR, 16'h0380);

						// Audip Clock Regeneratrion CTS Value, 44.1kHz@25.2/1.001Mhz = (31250) 0x7a12
						//18: write_i2c(CHIP_ADDR, 16'h0700);
						//19: write_i2c(CHIP_ADDR, 16'h087a);
						//20: write_i2c(CHIP_ADDR, 16'h0912);

						18: write_i2c(CHIP_ADDR, 16'h0B0E);
						19: write_i2c(CHIP_ADDR, 16'h0C85);
						20: write_i2c(CHIP_ADDR, 16'h0D10);
						21: write_i2c(CHIP_ADDR, 16'h5618);
						22: write_i2c(CHIP_ADDR, 16'h94C0);	// interrupt
						23: write_i2c(CHIP_ADDR, 16'h96C0);	// interrupt
					
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
