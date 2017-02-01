////////////////////////////////////////////////////////////////////////
// TMDS encoder
// Used to encode HDMI/DVI video data
////////////////////////////////////////////////////////////////////////

module TMDS_encoder(
	input clk,
	input [7:0] VD,  // video data (red, green or blue)
	input [1:0] CD,  // control data
	input VDE,  // video data enable, to choose between CD (when VDE=0) and VD (when VDE=1)
	output reg [9:0] TMDS
);

reg [3:0] balance_acc;
reg [8:0] q_m;
reg [1:0] CD2;
reg VDE2;

initial begin
	balance_acc=0;
	q_m=0;
	CD2=0;
	VDE2=0;
end

function [3:0] balance;
	input [7:0] qm;
	begin
		balance = qm[0] + qm[1] + qm[2] + qm[3] + qm[4] + qm[5] + qm[6] + qm[7] - 4'd4;
	end
endfunction

always @(posedge clk)
begin
	// Cycle 1
	if ((VD[0] + VD[1] + VD[2] + VD[3] + VD[4] + VD[5] + VD[6] + VD[7])>(VD[0]?4'd4:4'd3)) begin
		q_m <= {1'b0,~^VD[7:0],^VD[6:0],~^VD[5:0],^VD[4:0],~^VD[3:0],^VD[2:0],~^VD[1:0],VD[0]};
	end else begin
		q_m <= {1'b1, ^VD[7:0],^VD[6:0], ^VD[5:0],^VD[4:0], ^VD[3:0],^VD[2:0], ^VD[1:0],VD[0]};
	end
	VDE2 <= VDE;
	CD2 <= CD;
	// Cycle 2
	if (VDE2) begin
		if (balance(q_m)==0 || balance_acc==0) begin
			if (q_m[8]) begin
				TMDS <= {1'b0, q_m[8], q_m[7:0]};
				balance_acc <= balance_acc+balance(q_m);
			end else begin
				TMDS <= {1'b1, q_m[8], ~q_m[7:0]};
				balance_acc <= balance_acc-balance(q_m);
			end
		end else begin
			if (balance(q_m)>>3 == balance_acc[3]) begin
				TMDS <= {1'b1, q_m[8], ~q_m[7:0]};
				balance_acc <= balance_acc+q_m[8]-balance(q_m);
			end else begin
				TMDS <= {1'b0, q_m[8], q_m[7:0]};
				balance_acc <= balance_acc-(~q_m[8])+balance(q_m);
			end
		end
	end else begin
		balance_acc <= 0;
		TMDS <= CD2[1] ? (CD2[0] ? 10'b1010101011 : 10'b0101010100) : (CD2[0] ? 10'b0010101011 : 10'b1101010100);
	end
end

endmodule
