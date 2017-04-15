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
	
	//output [11:0] counterX,
	//output [11:0] counterY,
	output DrawArea,
	output videoClock
	
	
	/*,
	output [11:0] hpixels*/
);

	localparam hpixels  = 800; // horizontal pixels per line
	localparam hpxv 	  = 640; // horizontal pixels visible
	localparam hpstart  = 656; // beginning of horizontal pulse
	localparam hpulse   =  96; // hsync pulse length

	reg [9:0]  vlines; 			// vertical lines per frame
	localparam vlnsv	  = 480; // vertical lines visible
	localparam vpstart  = 490; // beginning of vertical pulse
	localparam vpulse   =   2; // vsync pulse length

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
	
	//reg [11:0] hpixels_reg;
	//reg [11:0] hpixels_reg_q;
	
	reg trigger = 1'b0;
	reg line_doubler_reg = 1'b0;
	reg add_line_reg = 1'b0;
	
	initial begin
		counterX_reg <= 0;
		counterY_reg <= 0;
		hsync_reg_q <= 1'b1;
		vsync_reg_q <= 1'b1;
		//hpixels_reg <= hpixels;
	end
	
	task doReset;
		input triggered;
	
		begin
			trigger <= triggered;
			counterX_reg <= 0;
			counterY_reg <= 0;
			hsync_reg_q <= 1'b1;
			vsync_reg_q <= 1'b1;
		end
	endtask	

	always @(*) begin
		if (add_line) begin
			vlines = 10'd526;
		end else begin
			vlines = 10'd525;
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
					hsync_reg_q <= 1'b0;
				end else begin
					hsync_reg_q <= 1'b1;
				end

				if (counterY_reg_q >= vpstart && counterY_reg_q < vpstart + vpulse + 1) begin // + 1: synchronize last vsync period with hsync negative edge
					if ((counterY_reg_q == vpstart && counterX_reg_q < hpstart) 
					 || (counterY_reg_q == vpstart + vpulse && counterX_reg_q >= hpstart)) begin
						vsync_reg_q <= 1;
					end else begin
						vsync_reg_q <= 0;
					end
				end else begin
					vsync_reg_q <= 1;
				end
				
				counterX_reg_q <= counterX_reg;
				counterY_reg_q <= counterY_reg;
				counterX_reg_q_q <= counterX_reg_q;
				counterY_reg_q_q <= counterY_reg_q;
			end
		end
	end
	
	assign rdaddr = (line_doubler_reg ? { counterY_reg[2:1], counterX_reg[9:0] } : counterX_reg[11:0]);
	assign red = rddata[31:24];
	assign green = rddata[23:16];
	assign blue = rddata[15:8];
	assign hsync = hsync_reg_q;
	assign vsync = vsync_reg_q;
	assign DrawArea = counterX_reg_q_q >= 0 && counterX_reg_q_q < hpxv && counterY_reg_q_q >= 0 && counterY_reg_q_q < vlnsv;
	assign videoClock = clock; 

endmodule