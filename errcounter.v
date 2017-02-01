module errcounter (input locked, output reg [2:0] counter);

	always @ (negedge locked)// on positive clock edge
	begin
		counter <= #1 counter + 1'b1;// increment counter
	end
endmodule// end of module counter
	