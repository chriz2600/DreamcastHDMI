module ram2video(
	input [31:0] rddata,
	input starttrigger,

	input clock,
	input reset,

	output [9:0] rdaddr,
	
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

	localparam vlines   = 525; // vertical lines per frame
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
	
	initial begin
		counterX_reg <= 0;
		counterY_reg <= 0;
		hsync_reg_q <= 1'b1;
		vsync_reg_q <= 1'b1;
		//hpixels_reg <= hpixels;
	end
	
	always @(posedge clock) begin
		if (~reset) begin
			trigger <= 1'b0;
			counterX_reg <= 0;
			counterY_reg <= 0;
			hsync_reg_q <= 1'b1;
			vsync_reg_q <= 1'b1;
		end else begin
			if (!trigger && starttrigger) begin
				trigger <= 1'b1;
				counterX_reg <= 0;
				counterY_reg <= 0;
				hsync_reg_q <= 1'b1;
				vsync_reg_q <= 1'b1;
			end
		
			if (trigger) begin
				
				if (counterX_reg < hpixels - 1) begin
					counterX_reg <= counterX_reg + 1;
				end else begin
					counterX_reg <= 0;
				
					if (counterY_reg < vlines - 1) begin
						counterY_reg <= counterY_reg + 1;
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
	
	assign rdaddr = counterX_reg;
	assign red = rddata[31:24];
	assign green = rddata[23:16];
	assign blue = rddata[15:8];
	assign hsync = hsync_reg_q;
	assign vsync = vsync_reg_q;
	assign DrawArea = counterX_reg_q_q >= 0 && counterX_reg_q_q < hpxv && counterY_reg_q_q >= 0 && counterY_reg_q_q < vlnsv;
	assign videoClock = clock; 

	/*
		if (counterX_reg < hpixels - 1) begin
			counterX_reg <= counterX_reg + 1;
		end else begin
			counterX_reg <= 0;
		
			if (counterY_reg < vlines - 1) begin
				counterY_reg <= counterY_reg + 1;
			end else begin
				counterY_reg <= 0;
			end
		end
		
		if (counterX_reg >= hpstart && counterX_reg < hpstart + hpulse) begin
			hsync_reg_q <= 1'b0;
		end else begin
			hsync_reg_q <= 1'b1;
		end

		if (counterY_reg >= vpstart && counterY_reg < vpstart + vpulse + 1) begin // + 1: synchronize last vsync period with hsync negative edge
			if ((counterY_reg == vpstart && counterX_reg < hpstart) 
			 || (counterY_reg == vpstart + vpulse && counterX_reg >= hpstart)) begin
				vsync_reg_q <= 1;
			end else begin
				vsync_reg_q <= 0;
			end
		end else begin
			vsync_reg_q <= 1;
		end

		if (counterX_reg >= 0 && counterX_reg < hpxv 
		 && counterY_reg >= 0 && counterY_reg < vlnsv) begin
		 
			if (counterY_reg > 159 && counterY_reg < 320 && counterX_reg < 440 && counterX_reg > 199) begin
				red_reg <= 8'd255;
				green_reg <= 8'd255;
				blue_reg <= 8'd255;
				
				if ((480 * counterY_reg - counterY_reg * counterY_reg + 640 * counterX_reg - counterX_reg * counterX_reg) > 157696) begin
              blue_reg <= 8'b00010011; 
				  green_reg <= 8'd0; 
				  red_reg <= 8'b10111110;
            end
			end else begin
				red_reg <= 8'd255;
				green_reg <= 8'd255;
				blue_reg <= 8'd255;
			end
		end else begin
			red_reg <= 8'd0;
			green_reg <= 8'd0;
			blue_reg <= 8'd0;
		end
		
		counterX_reg_q <= counterX_reg;
		counterY_reg_q <= counterY_reg;
		hpixels_reg_q <= hpixels_reg;
	end
	
	assign hsync = hsync_reg_q;
	assign vsync = vsync_reg_q;
	//assign counterX = counterX_reg_q;
	//assign counterY = counterY_reg_q;
	assign DrawArea = counterX_reg_q >= 0 && counterX_reg_q < hpxv && counterY_reg_q >= 0 && counterY_reg_q < vlnsv;
	assign red = red_reg;
	assign green = green_reg;
	assign blue = blue_reg;
	assign videoClock = clock; 
	//assign hpixels = hpixels_reg_q;
	*/

endmodule