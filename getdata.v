module getdata(
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

reg hsync_reg;
reg hsync_reg_q;
reg vsync_reg;
reg vsync_reg_q;

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

//localparam VISIBLE_AREA_START = 217;
localparam VISIBLE_AREA_START = 217 + 40;

always @(negedge clock) begin

	hsync_reg <= _hsync;
	vsync_reg <= _vsync;
	
	if (hsync_reg && !_hsync) begin
		raw_counterX_reg <= 0;
	end else begin
		raw_counterX_reg <= raw_counterX_reg + 1;
	end
	
	if (vsync_reg && !_vsync) begin
		raw_counterY_reg <= 0;
	end else if (hsync_reg && !_hsync) begin
		raw_counterY_reg <= raw_counterY_reg + 1;
	end
	
	if (raw_counterX_reg == VISIBLE_AREA_START) begin
		counterX_reg <= 0;
	end else begin
		counterX_reg <= counterX_reg + raw_counterX_reg[0];
	end

	if (raw_counterX_reg == VISIBLE_AREA_START) begin
		if (raw_counterY_reg == 40) begin
			counterY_reg <= 0;
		end else begin
			counterY_reg <= counterY_reg + 1;
		end
	end

	if (counterX_reg >= 0 && counterX_reg < 720 && counterY_reg >= 0 && counterY_reg < 480) begin
		if (raw_counterX_reg[0]) begin
			red_reg_buf <= indata[11:4];
			green_reg_buf[7:4] <= indata[3:0];
		end else begin
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
	hsync_reg_q <= hsync_reg;
	vsync_reg_q <= vsync_reg;
end

assign hsync = ~hsync_reg_q;
assign vsync = ~vsync_reg_q;
assign counterX = counterX_reg_q;
assign counterY = counterY_reg_q;
assign DrawArea = counterX_reg_q >= 0 && counterX_reg_q < 720 && counterY_reg_q >= 0 && counterY_reg_q < 480;
assign red = red_reg;
assign green = green_reg;
assign blue = blue_reg;

endmodule