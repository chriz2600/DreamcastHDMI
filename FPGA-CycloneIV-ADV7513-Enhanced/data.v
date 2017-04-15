module data(
	input clock,
	input reset,
	input [11:0] indata,
	input _hsync,
	input _vsync,
	input line_doubler,

	output clock_out,
	
	output [7:0] red,
	output [7:0] green,
	output [7:0] blue,
	
	output [11:0] counterX,
	output [11:0] counterY,
	output add_line
);

	reg hsync_reg;
	reg vsync_reg;

	reg [7:0] red_reg_buf;
	reg [7:0] red_reg;
	reg [7:0] green_reg_buf;
	reg [7:0] green_reg;
	reg [7:0] blue_reg;

	reg [11:0] raw_counterX_reg;
	reg [11:0] counterX_reg;
	reg [11:0] counterX_reg_q;

	reg [11:0] raw_counterY_reg;
	reg [11:0] counterY_reg;
	reg [11:0] counterY_reg_q;

	reg [9:0] VISIBLE_AREA_HSTART;
	reg [9:0] VISIBLE_AREA_VSTART;
	reg [9:0] VISIBLE_AREA_WIDTH;
	reg [9:0] VISIBLE_AREA_HEIGHT;
	
	reg add_line_reg;
	
	always @(*) begin
		if (line_doubler) begin
			if (add_line) begin
				VISIBLE_AREA_HSTART = 10'd347;
				VISIBLE_AREA_VSTART = 10'd18;
				VISIBLE_AREA_WIDTH  = 10'd643;
				VISIBLE_AREA_HEIGHT = 10'd504;
			end else begin
				VISIBLE_AREA_HSTART = 10'd327;
				VISIBLE_AREA_VSTART = 10'd18;
				VISIBLE_AREA_WIDTH  = 10'd643;
				VISIBLE_AREA_HEIGHT = 10'd504;
			end
		end else begin
			VISIBLE_AREA_HSTART = 10'd257;
			VISIBLE_AREA_VSTART = 10'd40;
			VISIBLE_AREA_WIDTH  = 10'd720;
			VISIBLE_AREA_HEIGHT = 10'd480;
		end
	end
	
	
	always @(negedge clock) begin
		hsync_reg <= _hsync;
		vsync_reg <= _vsync;
		
		// reset horizontal raw counter on hsync
		if (hsync_reg && !_hsync) begin
			raw_counterX_reg <= 0;
			
			// reset vertical raw counter on vsync
			if (vsync_reg && !_vsync) begin
				// 240p has only 263 lines per frame
				if (raw_counterY_reg == 262) begin
					add_line_reg <= 1'b1;
				end else begin
					add_line_reg <= 1'b0;
				end
				
				raw_counterY_reg <= 0;
			end else begin
				raw_counterY_reg <= raw_counterY_reg + 1'b1;
			end
		end else begin
			raw_counterX_reg <= raw_counterX_reg + 1'b1;
		end
		
		// recalculate counterX and counterY to match visible area
		if (raw_counterX_reg == VISIBLE_AREA_HSTART) begin
			counterX_reg <= 0;
			
			if (raw_counterY_reg == VISIBLE_AREA_VSTART) begin
				counterY_reg <= 0;
			end else begin
				counterY_reg <= counterY_reg + 1'b1;
			end
		end else begin
			counterX_reg <= counterX_reg + raw_counterX_reg[0];
		end

		// store red and first half of green
		if (counterX_reg >= 0 && counterX_reg < VISIBLE_AREA_WIDTH 
		 && counterY_reg >= 0 && counterY_reg < VISIBLE_AREA_HEIGHT) begin
			// store values on even clock
			if (raw_counterX_reg[0]) begin
				red_reg_buf <= indata[11:4];
				green_reg_buf[7:4] <= indata[3:0];
			end else begin
				// apply combined values of red, green, blue simultanesly
				red_reg <= red_reg_buf;
				green_reg <= { green_reg_buf[7:4], indata[11:8] };
				blue_reg <= indata[7:0];
			end
		end else begin
			red_reg <= 8'd0;
			green_reg <= 8'd0;
			blue_reg <= 8'd0;
		end

		counterX_reg_q <= counterX_reg;
		counterY_reg_q <= counterY_reg;
	end

	assign counterX = counterX_reg_q;
	assign counterY = counterY_reg_q;
	assign red = red_reg;
	assign green = green_reg;
	assign blue = blue_reg;
	assign clock_out = ~raw_counterX_reg[0];
	assign add_line = add_line_reg;

endmodule