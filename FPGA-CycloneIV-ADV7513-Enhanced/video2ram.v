module video2ram(
	input clock,
	
	input [7:0] R,
	input [7:0] G,
	input [7:0] B,
	input [11:0] counterX,
	input [11:0] counterY,
	
	input line_doubler,
	
	output [31:0] wrdata,
	output [11:0] wraddr,
	output wren,
	output wrclock,
	
	output starttrigger
);

	reg [9:0] H_CAPTURE_START = 44;
	reg [9:0] H_CAPTURE_END   = 684;
	reg [9:0] H_TRIGGER_POINT = 342;
	reg [9:0] V_CAPTURE_START = 0;
	reg [9:0] V_CAPTURE_END   = 480;
	reg [9:0] V_TRIGGER_POINT = 0;

	reg wren_reg = 0;
	reg [31:0] wrdata_reg;
	reg [11:0] wraddr_reg;
	reg [11:0] tmp;
	reg trigger = 0;

	always @(*) begin
		if (line_doubler) begin
			H_CAPTURE_START = 10'd1;
			H_CAPTURE_END   = 10'd641;
			H_TRIGGER_POINT = 10'd320;
			V_CAPTURE_START = 10'd0;
			V_CAPTURE_END   = 10'd504;
			V_TRIGGER_POINT = 10'd1;
		end else begin
			H_CAPTURE_START = 10'd44;
			H_CAPTURE_END   = 10'd684;
			H_TRIGGER_POINT = 10'd320;
			V_CAPTURE_START = 10'd0;
			V_CAPTURE_END   = 10'd480;
			V_TRIGGER_POINT = 10'd0;
		end
	end
	
	
	always @ (posedge clock) begin
	
		if (line_doubler) begin
			// 480i/240p mode
			if (counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
				wraddr_reg[9:0] <= counterX[9:0] - H_CAPTURE_START;
				
				if (counterY < 240) begin
					wren_reg <= 1;
					wraddr_reg[11:10] <= counterY[1:0];
					wrdata_reg <= { R, G, B, 8'd0 };
				end else if (counterY > 262 && counterY < 504) begin
					tmp = counterY - 12'd3;
					wren_reg <= 1;
					wraddr_reg[11:10] <= tmp[1:0];
					wrdata_reg <= { R, G, B, 8'd0 };
				end else begin
					wren_reg <= 0;
				end
				
				if (counterX == H_TRIGGER_POINT && counterY == V_TRIGGER_POINT) begin
					trigger <= 1'b1;
				end
			end else begin
				wren_reg <= 0;
				trigger <= 1'b0;
			end

		end else begin
			// 480p mode
			if (counterY >= V_CAPTURE_START && counterY < V_CAPTURE_END && counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
				wren_reg <= 1;
				wraddr_reg <= counterX - 12'd44;
				wrdata_reg <= { R, G, B, 8'd0 };
				
				if (counterX == H_TRIGGER_POINT && counterY == V_TRIGGER_POINT) begin
					trigger <= 1'b1;
				end else begin
					trigger <= 1'b0;
				end
			end else begin
				wren_reg <= 0;
				wraddr_reg <= 12'd1023;
				wrdata_reg <= 32'd0;
				trigger <= 1'b0;
			end
		end
	end

	assign wren = wren_reg;
	assign wrclock = clock;
	assign wraddr = wraddr_reg;
	assign wrdata = wrdata_reg;
	assign starttrigger = trigger;
	
	
endmodule