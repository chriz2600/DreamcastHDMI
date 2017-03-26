//Written by: Holguer A Becerra
//ECE 31289 UPB-2014-I
//Semillero ADT UPB Bucaramanga
//Based on FPGA4FUN examples


module simple_serializer(
	input pclkx5,
	input pclkx5_not,
	input [9:0]TMDS_red,
	input [9:0]TMDS_green,
	input [9:0]TMDS_blue,
	output reg [1:0]ser_red,
	output reg [1:0]ser_green,
	output reg [1:0]ser_blue
);


	reg [3:0] TMDS_counter; 
	always @(posedge pclkx5) TMDS_counter <= (TMDS_counter == 4) ? 4'b0000 : TMDS_counter + 1'b1;

	reg TMDS_shift_load;
	always @(posedge pclkx5) TMDS_shift_load <= (TMDS_counter==4);

	reg [9:0] TMDS_shift_red, TMDS_shift_green, TMDS_shift_blue;
	always @(posedge pclkx5)
	begin
		 TMDS_shift_red   <= TMDS_shift_load ? TMDS_red   : TMDS_shift_red  [9:2];
		 TMDS_shift_green <= TMDS_shift_load ? TMDS_green : TMDS_shift_green[9:2];
		 TMDS_shift_blue  <= TMDS_shift_load ? TMDS_blue  : TMDS_shift_blue [9:2];	
	end


	always@(posedge pclkx5_not)
	begin
		ser_red[1:0]<=TMDS_shift_red[1:0];
		ser_green[1:0]<=TMDS_shift_green[1:0];
		ser_blue[1:0]<=TMDS_shift_blue[1:0];
	end

endmodule

