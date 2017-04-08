module video2ram(
	input clock,
	
	input [7:0] R,
	input [7:0] G,
	input [7:0] B,
	input [11:0] counterX,
	input [11:0] counterY,
	
	output [31:0] wrdata,
	output [9:0] wraddr,
	output wren,
	output wrclock,
	
	output starttrigger
);

	localparam H_CAPTURE_START = 44;
	localparam H_CAPTURE_END   = 684;
	localparam H_TRIGGER_POINT = 342;
	
	localparam V_CAPTURE_START = 0;
	localparam V_CAPTURE_END   = 480;
	localparam V_TRIGGER_POINT = 0;

	reg wren_reg = 0;
	reg [31:0] wrdata_reg;
	reg [31:0] wraddr_reg;
	reg trigger = 0;

	always @ (posedge clock) begin
	
		if (counterY >= V_CAPTURE_START && counterY < V_CAPTURE_END && counterX >= H_CAPTURE_START && counterX < H_CAPTURE_END) begin
			wren_reg <= 1;
			wraddr_reg <= counterX - 44;
			wrdata_reg <= { R, G, B, 8'd0 };
			
			if (counterX == H_TRIGGER_POINT && counterY == V_TRIGGER_POINT) begin
				trigger <= 1'b1;
			end
		end else begin
			wren_reg <= 0;
			wraddr_reg <= 9'd1023;
			wrdata_reg <= 32'd0;
			trigger <= 1'b0;
		end
	
	end

	assign wren = wren_reg;
	assign wrclock = clock;
	assign wraddr = wraddr_reg;
	assign wrdata = wrdata_reg;
	assign starttrigger = trigger;
	
	
endmodule