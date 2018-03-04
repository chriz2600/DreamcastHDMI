module ram2video(
	input [31:0] rddata,
	input starttrigger,

	input clock,
	input reset,
	
	input line_doubler,
	input add_line,

	output [11:0] rdaddr,
	
	output [7:0] red,
	output [7:0] green,
	output [7:0] blue,
	
	output hsync,
	output vsync,
	
	output DrawArea,
	output videoClock
);

	// VGA
	localparam hpixels  =  800; // horizontal pixels per line
	localparam hpxv 	  =  640; // horizontal pixels visible
	localparam hpstart  =  656; // beginning of horizontal pulse
	localparam hpulse   =   96; // hsync pulse length
	localparam _vlines  =  525; // vertical lines per frame
	localparam _vlinesI =  526; // vertical lines per frame interlaced
	localparam vlnsv	  =  480; // vertical lines visible
	localparam vpstart  =  490; // beginning of vertical pulse
	localparam vpulse   =    2; // vsync pulse length
	localparam VSYNC_ON =    0; // polarity of vertical sync pulse
	localparam HSYNC_ON =    0; // polarity of horizontal sync pulse
	localparam H_OFFSET =    0; // horizontal pixel offset
	localparam PXL_FACTOR =  1; // pixel factor
	
	/*
	// 1080p
	localparam hpixels  = 2200; // horizontal pixels per line
	localparam hpxv 	  = 1920; // horizontal pixels visible
	localparam hpstart  = 2008; // beginning of horizontal pulse
	localparam hpulse   =   44; // hsync pulse length
	localparam _vlines  = 1125; // vertical lines per frame
	localparam _vlinesI = 1126; // vertical lines per frame interlaced
	localparam vlnsv	  = 1080; // vertical lines visible
	localparam vpstart  = 1084; // beginning of vertical pulse
	localparam vpulse   =    5; // vsync pulse length
	localparam VSYNC_ON =    1; // polarity of vertical sync pulse
	localparam HSYNC_ON =    1; // polarity of horizontal sync pulse
	localparam H_OFFSET =  240; // horizontal pixel offset
	localparam PXL_FACTOR =  2; // pixel factor
	*/

	/*	
	// 1280 x 960 @ 60Hz;
	localparam hpixels  = 1800; // horizontal pixels per line
	localparam hpxv 	  = 1280; // horizontal pixels visible
	localparam hpstart  = 1376; // beginning of horizontal pulse
	localparam hpulse   =  112; // hsync pulse length
	localparam _vlines  = 1000; // vertical lines per frame
	localparam _vlinesI = 1000; // vertical lines per frame interlaced
	localparam vlnsv	  =  960; // vertical lines visible
	localparam vpstart  =  961; // beginning of vertical pulse
	localparam vpulse   =    3; // vsync pulse length
	localparam VSYNC_ON =    1; // polarity of vertical sync pulse
	localparam HSYNC_ON =    1; // polarity of horizontal sync pulse
	localparam H_OFFSET =    0; // horizontal pixel offset
	localparam PXL_FACTOR =  2; // pixel factor
	*/
	
	reg [10:0]  vlines; 			 // vertical lines per frame (get set from _vlines or _vlinesI)
	
	reg [7:0] red_reg;
	reg [7:0] green_reg;
	reg [7:0] blue_reg;

	reg hsync_reg_q = 1'b1;
	reg vsync_reg_q = 1'b1;

	reg [11:0] counterX_reg;
	reg [11:0] counterX_reg_q;
	reg [11:0] counterX_reg_q_q;
	
	reg [11:0] counterY_reg;
	reg [11:0] counterY_reg_q;
	reg [11:0] counterY_reg_q_q;
	
	reg trigger = 1'b0;
	reg line_doubler_reg = 1'b0;
	reg add_line_reg = 1'b0;
	
	initial begin
		counterX_reg <= 0;
		counterY_reg <= 0;
		hsync_reg_q <= ~HSYNC_ON;
		vsync_reg_q <= ~VSYNC_ON;
	end
	
	task doReset;
		input triggered;
	
		begin
			trigger <= triggered;
			counterX_reg <= 0;
			counterY_reg <= 0;
			hsync_reg_q <= ~HSYNC_ON;
			vsync_reg_q <= ~VSYNC_ON;
		end
	endtask	

	always @(*) begin
		if (add_line) begin
			vlines = _vlinesI;
		end else begin
			vlines = _vlines;
		end
	end
	
	always @(posedge clock) begin
		if (~reset) begin
			doReset(1'b0);
		end else begin
			if (!trigger && starttrigger) begin
				doReset(1'b1);
			end
			
			line_doubler_reg <= line_doubler;
			if (line_doubler != line_doubler_reg) begin
				doReset(1'b0);
			end

			add_line_reg <= add_line;
			if (add_line != add_line_reg) begin
				doReset(1'b0);
			end
		
			if (trigger) begin
				
				if (counterX_reg < hpixels - 1) begin
					counterX_reg <= counterX_reg + 1'b1;
				end else begin
					counterX_reg <= 0;
				
					if (counterY_reg < vlines - 1) begin
						counterY_reg <= counterY_reg + 1'b1;
					end else begin
						counterY_reg <= 0;
					end
				end

				if (counterX_reg_q >= hpstart && counterX_reg_q < hpstart + hpulse) begin
					hsync_reg_q <= HSYNC_ON;
				end else begin
					hsync_reg_q <= ~HSYNC_ON;
				end

				if (counterY_reg_q >= vpstart && counterY_reg_q < vpstart + vpulse + 1) begin // + 1: synchronize last vsync period with hsync negative edge
					if ((counterY_reg_q == vpstart && counterX_reg_q < hpstart) 
					 || (counterY_reg_q == vpstart + vpulse && counterX_reg_q >= hpstart)) begin
						vsync_reg_q <= VSYNC_ON;
					end else begin
						vsync_reg_q <= ~VSYNC_ON;
					end
				end else begin
					vsync_reg_q <= VSYNC_ON;
				end
				
				counterX_reg_q <= counterX_reg;
				counterY_reg_q <= counterY_reg;
				counterX_reg_q_q <= counterX_reg_q;
				counterY_reg_q_q <= counterY_reg_q;
			end
		end
	end
	
	assign rdaddr = (
		counterX_reg >= H_OFFSET 
		? (
			line_doubler_reg 
			? { counterY_reg[2:1], counterX_reg[9:0] - (H_OFFSET / PXL_FACTOR) } 
			: { 1'b0, (PXL_FACTOR == 2 ? counterX_reg[11:1] : counterX_reg[10:0]) - (H_OFFSET / PXL_FACTOR) }
		) 
		: 12'd1023
	);
	assign red = rddata[31:24];
	assign green = rddata[23:16];
	assign blue = rddata[15:8];
	assign hsync = hsync_reg_q;
	assign vsync = vsync_reg_q;
	assign DrawArea = counterX_reg_q_q >= 0 && counterX_reg_q_q < hpxv && counterY_reg_q_q >= 0 && counterY_reg_q_q < vlnsv;
	assign videoClock = clock; 

endmodule