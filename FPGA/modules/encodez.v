`timescale 1 ps / 1ps

module encodez(
	input             clkin,    // pixel clock input
	input             rstin,    // async. reset input (active high)
	input      [7:0]  din,      // data inputs: expect registered
	input             c0,       // c0 input
	input             c1,       // c1 input
	input      [11:0] counterX, // x position
	input      [11:0] counterY, // y position
	input             de,       // de input

	input 				iDataD0,
	input 	  			iDataD1,
	input 	  			iDataD2,
	input 	  			iDataD3,

	output reg [9:0]  dout      // data outputs
);
	parameter CHANNEL = "BLUE";

	localparam VIDEOGBND = (CHANNEL == "BLUE") ?  10'b1011001100 :
								  (CHANNEL == "GREEN") ? 10'b0100110011 : 10'b1011001100;

	localparam DILNDGBND = (CHANNEL == "GREEN") ? 10'b0100110011 :
								  (CHANNEL == "RED") ?   10'b0100110011 : 10'bxxxxxxxxxx;

	function [9:0] TERC4encode;
		input [3:0] data;
		begin
			case (data)
				4'b0000: TERC4encode = 10'b1010011100;
				4'b0001: TERC4encode = 10'b1001100011;
				4'b0010: TERC4encode = 10'b1011100100;
				4'b0011: TERC4encode = 10'b1011100010;
				4'b0100: TERC4encode = 10'b0101110001;
				4'b0101: TERC4encode = 10'b0100011110;
				4'b0110: TERC4encode = 10'b0110001110;
				4'b0111: TERC4encode = 10'b0100111100;
				4'b1000: TERC4encode = 10'b1011001100;
				4'b1001: TERC4encode = 10'b0100111001;
				4'b1010: TERC4encode = 10'b0110011100;
				4'b1011: TERC4encode = 10'b1011000110;
				4'b1100: TERC4encode = 10'b1010001110;
				4'b1101: TERC4encode = 10'b1001110001;
				4'b1110: TERC4encode = 10'b0101100011;
				4'b1111: TERC4encode = 10'b1011000011;
			endcase
		end
	endfunction

	function [9:0] CTLencode;
		input [1:0] data;
		begin
			case (data)
				2'b00: CTLencode = 10'b1101010100;
				2'b01: CTLencode = 10'b0010101011;
				2'b10: CTLencode = 10'b0101010100;
				2'b11: CTLencode = 10'b1010101011;
			endcase
		end
	endfunction

	////////////////////////////////////////////////////////////
	// Counting number of 1s and 0s for each incoming pixel
	// component. Pipe line the result.
	// Register Data Input so it matches the pipe lined adder
	// output
	////////////////////////////////////////////////////////////
	reg [3:0] n1d; //number of 1s in din
	reg [7:0] din_q;

	always @ (posedge clkin) begin
		n1d <=#1 din[0] + din[1] + din[2] + din[3] + din[4] + din[5] + din[6] + din[7];
		din_q <=#1 din;
	end

	///////////////////////////////////////////////////////
	// Stage 1: 8 bit -> 9 bit
	// Refer to DVI 1.0 Specification, page 29, Figure 3-5
	///////////////////////////////////////////////////////
	wire decision1;

	assign decision1 = (n1d > 4'h4) | ((n1d == 4'h4) & (din_q[0] == 1'b0));

	wire [8:0] q_m;
	assign q_m[0] = din_q[0];
	assign q_m[1] = (decision1) ? (q_m[0] ^~ din_q[1]) : (q_m[0] ^ din_q[1]);
	assign q_m[2] = (decision1) ? (q_m[1] ^~ din_q[2]) : (q_m[1] ^ din_q[2]);
	assign q_m[3] = (decision1) ? (q_m[2] ^~ din_q[3]) : (q_m[2] ^ din_q[3]);
	assign q_m[4] = (decision1) ? (q_m[3] ^~ din_q[4]) : (q_m[3] ^ din_q[4]);
	assign q_m[5] = (decision1) ? (q_m[4] ^~ din_q[5]) : (q_m[4] ^ din_q[5]);
	assign q_m[6] = (decision1) ? (q_m[5] ^~ din_q[6]) : (q_m[5] ^ din_q[6]);
	assign q_m[7] = (decision1) ? (q_m[6] ^~ din_q[7]) : (q_m[6] ^ din_q[7]);
	assign q_m[8] = (decision1) ? 1'b0 : 1'b1;

	/////////////////////////////////////////////////////////
	// Stage 2: 9 bit -> 10 bit
	// Refer to DVI 1.0 Specification, page 29, Figure 3-5
	/////////////////////////////////////////////////////////
	reg [3:0] n1q_m, n0q_m; // number of 1s and 0s for q_m
	always @ (posedge clkin) begin
		n1q_m  <=#1 q_m[0] + q_m[1] + q_m[2] + q_m[3] + q_m[4] + q_m[5] + q_m[6] + q_m[7];
		n0q_m  <=#1 4'h8 - (q_m[0] + q_m[1] + q_m[2] + q_m[3] + q_m[4] + q_m[5] + q_m[6] + q_m[7]);
	end

	reg [4:0] cnt; //disparity counter, MSB is the sign bit
	wire decision2, decision3;

	assign decision2 = (cnt == 5'h0) | (n1q_m == n0q_m);
	/////////////////////////////////////////////////////////////////////////
	// [(cnt > 0) and (N1q_m > N0q_m)] or [(cnt < 0) and (N0q_m > N1q_m)]
	/////////////////////////////////////////////////////////////////////////
	assign decision3 = (~cnt[4] & (n1q_m > n0q_m)) | (cnt[4] & (n0q_m > n1q_m));

	////////////////////////////////////
	// pipe line alignment
	////////////////////////////////////
	reg        de_q, de_reg;
	reg        c0_q, c1_q;
	reg        c0_reg, c1_reg;
	reg [11:0] counterX_q, counterX_reg;
	//reg [11:0] counterY_q, counterY_reg;
	//reg [11:0] dataCounter;
	reg [8:0]  q_m_reg;

	always @ (posedge clkin) begin
		de_q    <=#1 de;
		de_reg  <=#1 de_q;

		c0_q    <=#1 c0;
		c0_reg  <=#1 c0_q;
		c1_q    <=#1 c1;
		c1_reg  <=#1 c1_q;

		counterX_q   <=#1 counterX;
		counterX_reg <=#1 counterX_q;
		//dataCounter	 <=#1 counterX_q - 751;
		//counterY_q   <=#1 counterY;
		//counterY_reg <=#1 counterY_q;

		q_m_reg <=#1 q_m;
	end

	///////////////////////////////
	// 10-bit out
	// disparity counter
	///////////////////////////////
	always @ (posedge clkin or posedge rstin) begin
		if(rstin) begin
			dout <= 10'h0;
			cnt <= 5'h0;
		end else begin
			if (de_reg) begin
				if(decision2) begin
					dout[9]   <=#1 ~q_m_reg[8];
					dout[8]   <=#1 q_m_reg[8];
					dout[7:0] <=#1 (q_m_reg[8]) ? q_m_reg[7:0] : ~q_m_reg[7:0];

					cnt <=#1 (~q_m_reg[8]) ? (cnt + n0q_m - n1q_m) : (cnt + n1q_m - n0q_m);
				end else begin
					if(decision3) begin
						dout[9]   <=#1 1'b1;
						dout[8]   <=#1 q_m_reg[8];
						dout[7:0] <=#1 ~q_m_reg[7:0];

						cnt <=#1 cnt + {q_m_reg[8], 1'b0} + (n0q_m - n1q_m);
					end else begin
						dout[9]   <=#1 1'b0;
						dout[8]   <=#1 q_m_reg[8];
						dout[7:0] <=#1 q_m_reg[7:0];

						cnt <=#1 cnt - {~q_m_reg[8], 1'b0} + (n1q_m - n0q_m);
					end
				end
			end else begin
				if (counterX_reg >= 741 && counterX_reg < 817) begin // data island
					// preamble 8 px GREEN and RED
					if (counterX_reg < 749) begin
						if (CHANNEL == "GREEN") begin
							// ctl0, ctl1 1,0
							dout <=#1 CTLencode({ 1'b0, 1'b1 });
						end else if (CHANNEL == "RED") begin
							// ctl2, ctl3 1,0
							dout <=#1 CTLencode({ 1'b0, 1'b1 });
						end else begin// CHANNEL == "BLUE"
							dout <=#1 CTLencode({ c1_reg, c0_reg });
						end
					// guard band 2 px
					end else if (counterX_reg < 751) begin
						if (CHANNEL == "GREEN") begin
							dout <=#1 DILNDGBND;
						end else if (CHANNEL == "RED") begin
							// ctl2, ctl3 1,0
							dout <=#1 DILNDGBND;
						end else begin// CHANNEL == "BLUE"
							dout <= TERC4encode({1'b1, 1'b1, c1_reg, c0_reg});
						end
					// packet time 64 px
					end else if (counterX_reg < 815) begin
						dout <= TERC4encode({ iDataD3, iDataD2, iDataD1, iDataD0 });
					// guard band 2 px
					end else if (counterX_reg < 817) begin
						if (CHANNEL == "GREEN") begin
							dout <=#1 DILNDGBND;
						end else if (CHANNEL == "RED") begin
							// ctl2, ctl3 1,0
							dout <=#1 DILNDGBND;
						end else begin// CHANNEL == "BLUE"
							dout <= TERC4encode({1'b1, 1'b1, c1_reg, c0_reg});
						end
					end
				end else if (counterX_reg >= 856) begin // video guard band starts 2 pixels before video data period
					dout <=#1 VIDEOGBND;
				end else if (CHANNEL != "BLUE" && counterX_reg >= 848 && counterX_reg < 856) begin // no preamble on channel0 (BLUE)
					// Preamble for Video Data Period ctl[3:0] = 4'b0001
					if (CHANNEL == "GREEN") begin
						dout <=#1 CTLencode({ 1'b0, 1'b1 });
					end else begin// CHANNEL == "RED"
						dout <=#1 CTLencode({ 1'b0, 1'b0 });
					end
				end else begin
					dout <=#1 CTLencode({ c1_reg, c0_reg });
				end

				cnt <=#1 5'h0;
			end
		end
	end

endmodule
