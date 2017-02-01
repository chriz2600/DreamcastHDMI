module counter (input clock, output reg [31:0] counter);

	always @ (posedge clock)// on positive clock edge
	begin
		counter <= #1 counter + 1;// increment counter
	end
endmodule// end of module counter
	