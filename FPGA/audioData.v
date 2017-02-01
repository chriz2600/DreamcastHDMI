module audioData(

	input audioclk,
	input bckin,
	input lrcin,
	input data,
	input wrfull,
	input enable,

	output reg [31:0] dataOut,
	output reg wrreq
);
	reg lr;
	reg lrcin_reg;
	reg data_reg;
	reg write_data;
	reg [7:0] counter;
	reg [15:0] sampleLeft;
	reg [15:0] sampleRight;

	reg [15:0] sampleLeft_reg;
	reg [15:0] sampleRight_reg;

	always @(posedge audioclk) begin
		if (enable) begin
			lrcin_reg <= lrcin;
			data_reg <= data;

			if (!lrcin_reg && lrcin) begin
				lr <= 1'b1;
			end else if (lrcin_reg && !lrcin) begin
				lr <= 1'b0;
			end

			if (!lrcin_reg && lrcin) begin
				counter <= 0;
			end else begin
				counter <= counter + 1'b1;
			end

			if (counter == 32) begin
				dataOut <= { sampleLeft_reg, sampleRight_reg };
				write_data <= 1'b1;
			end else begin
				// store sample in fifo
				if (!wrfull && !wrreq && write_data) begin
					wrreq <= 1'b1;
					write_data <= 1'b0;
				end else begin
					wrreq <= 1'b0;
				end
			end
		end
	end

	always @(posedge bckin) begin
		if (enable) begin
			if (lr) begin
				sampleRight_reg <= sampleRight;
				// left channel
				sampleLeft <= { sampleLeft[14:0], data_reg };
			end else begin
				sampleLeft_reg <= sampleLeft;
				// right channel
				sampleRight <= sampleRight << 1 | data_reg;
			end
		end
	end
endmodule
