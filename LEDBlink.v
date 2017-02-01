module LEDBlink(

	//////////// CLOCK //////////
	input [31:0] counter54,
	input [31:0] counter27,
	input [31:0] counter27x5,
	input [31:0] counter27xnot5,
	input [31:0] counterAudio,

	//////////// LED //////////
	output [4:0] LED
);



//=======================================================
//  REG/WIRE declarations
//=======================================================

	assign LED[0] = counter54[25];
	assign LED[1] = counter27[24];
	assign LED[2] = counter27x5[26];
	assign LED[3] = ~counter27xnot5[26];
	assign LED[4] = counterAudio[23];
	
endmodule
