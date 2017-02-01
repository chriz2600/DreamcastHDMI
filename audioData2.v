module audioData2(

	input clock,
	input lrcin,
	
	input [15:0] sampleLeft,
	input [15:0] sampleRight,
	
	output sclk,
	output [15:0] left,
	output [15:0] right,
	output [11:0] counter
);
	reg lrcin_reg;
	reg slck_reg;
	reg slck_reg_q;
	reg [15:0] sampleLeft_reg;
	reg [15:0] sampleLeft_reg_q;
	reg [15:0] sampleRight_reg;
	reg [15:0] sampleRight_reg_q;
	reg [11:0] counter_reg;

	always @(posedge clock) begin
		lrcin_reg <= lrcin;
		
		// lcin changed
		if (lrcin_reg != lrcin) begin
			if (lrcin_reg) begin
				counter_reg <= 0;
				sampleLeft_reg <= sampleLeft;
			end else begin
				counter_reg <= counter_reg + 1;
				sampleRight_reg <= sampleRight;
			end
		end else begin
			counter_reg <= counter_reg + 1;
		end
		
		if (counter_reg == 32) begin
			sampleLeft_reg_q <= sampleLeft_reg;
			sampleRight_reg_q <= sampleRight_reg;
			slck_reg <= 1;
			slck_reg_q <= slck_reg;
		end else if (counter_reg == 0) begin
			slck_reg <= 0;
			slck_reg_q <= slck_reg;
		end
	end	
	
	assign sclk = slck_reg_q;
	assign left  = sampleLeft_reg_q;
	assign right = sampleRight_reg_q;
	assign counter = counter_reg;
	
endmodule
