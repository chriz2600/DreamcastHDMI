module readAudio(
	input rclock,
	input [31:0] data,
	input rdempty,
	
	output reg rdreq,
	output reg [33:0] dataOut
);
	reg [2:0] counter;
	reg [11:0] testCounter;
	reg [11:0] testCounter2;
	
	always @(posedge rclock) begin

		if (!rdempty && testCounter == 0) begin
			rdreq <= 1'b1;
		end else if (rdempty) begin
			rdreq <= 1'b0;
		end

		if (rdreq) begin
			counter <= counter + 1;
			if (counter > 0) begin
				dataOut <= { data, 1'b0 };
			end else begin
				dataOut <= { 32'b0, 1'b1 };
			end
		end else begin
			counter <= 0;
			dataOut <= { 32'b0, 1'b1 };
		end
		
		if (testCounter == 857) begin
			testCounter <= 0;
			testCounter2 <= testCounter2 + 1;
		end else begin
			testCounter <= testCounter + 1;
		end
		
		if (testCounter2 == 524) begin
			testCounter2 <= 0;
		end
	end

endmodule
	