module data(
	input clock,
	input [11:0] indata,
	input _hsync,
	input _vsync,

	output [7:0] red,
	output [7:0] green,
	output [7:0] blue,
	
	output hsync,
	output vsync,
	
	output [11:0] counterX,
	output [11:0] counterY,
	output DrawArea
);

parameter CEA_861_D_TIMING_CORRECTION = "TRUE";

reg hsync_reg;
reg hsync_reg_q = 1'b1;
reg vsync_reg;
reg vsync_reg_q = 1'b1;

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

localparam VISIBLE_AREA_HSTART = 217 + 40;
localparam VISIBLE_AREA_VSTART =  40;
localparam VISIBLE_AREA_WIDTH  = 720;
localparam VISIBLE_AREA_HEIGHT = 480;

localparam HSYNC_START = 736;
localparam HSYNC_WIDTH =  62;
localparam VSYNC_START = 483; // 488 according to CEA-861-D 4.5 Figure 4
localparam VSYNC_WIDTH =   6;

always @(negedge clock) begin

	hsync_reg <= _hsync;
	vsync_reg <= _vsync;
	
	// reset horizontal raw counter on hsync
	if (hsync_reg && !_hsync) begin
		raw_counterX_reg <= 0;
	end else begin
		raw_counterX_reg <= raw_counterX_reg + 1'b1;
	end
	
	// reset vertical raw counter on vsync
	if (vsync_reg && !_vsync) begin
		raw_counterY_reg <= 0;
	end else if (hsync_reg && !_hsync) begin
		raw_counterY_reg <= raw_counterY_reg + 1'b1;
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

	if (CEA_861_D_TIMING_CORRECTION == "TRUE") begin
		// reconstruct hsync according to CEA-861-D 4.5 Figure 4
		if (counterX_reg >= HSYNC_START && counterX_reg < HSYNC_START + HSYNC_WIDTH) begin
			hsync_reg_q <= 0;
		end else begin
			hsync_reg_q <= 1;
		end
		
		// reconstruct vsync according to CEA-861-D 4.5 Figure 4
		if (counterY_reg >= VSYNC_START && counterY_reg < VSYNC_START + VSYNC_WIDTH + 1) begin // + 1: synchronize last vsync period with hsync negative edge
			if ((counterY_reg == VSYNC_START && counterX_reg < HSYNC_START) 
			 || (counterY_reg == VSYNC_START + VSYNC_WIDTH && counterX_reg >= HSYNC_START)) begin
				vsync_reg_q <= 1;
			end else begin
				vsync_reg_q <= 0;
			end
		end else begin
			vsync_reg_q <= 1;
		end
	end else begin
		hsync_reg_q <= hsync_reg;
		vsync_reg_q <= vsync_reg;
	end
	
	counterX_reg_q <= counterX_reg;
	counterY_reg_q <= counterY_reg;
end

assign hsync = hsync_reg_q;
assign vsync = vsync_reg_q;
assign counterX = counterX_reg_q;
assign counterY = counterY_reg_q;
assign DrawArea = counterX_reg_q >= 0 && counterX_reg_q < VISIBLE_AREA_WIDTH && counterY_reg_q >= 0 && counterY_reg_q < VISIBLE_AREA_HEIGHT;
assign red = red_reg;
assign green = green_reg;
assign blue = blue_reg;

endmodule