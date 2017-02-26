//Written by: chriz2600
// Based on code by: Holguer A Becerra
// https://sites.google.com/site/ece31289upb/practicas-de-clase/practica-4-sincronizadores/hdmi_de0-nano
//ECE 31289 UPB-2014-I
//Semillero ADT UPB Bucaramanga
//Based on Xilinx examples

// includes for icarus-verilog checking
//`include "modules/encodez.v"
//`include "simple_serializer.v"
//`include "DDIO.v"

module HDMI_TMDS(

	//////////// CLOCK //////////
	input pixclk_global,
	input pixclkx5_global,
	input pixclkx5_not_global,
	input reset,

	input [7:0] red,
	input [7:0] green,
	input [7:0] blue,

	input hSync,
	input vSync,
	input [11:0] counterX,
	input [11:0] counterY,
	input DrawArea,

	input [31:0] audio_data,
	input audio_rdempty,

	output R,	// red
	output G,	// green
	output B,	// blue
	output C,	// clock

	output reg audio_rdreq
);


//=======================================================
//  task/functions
//=======================================================

	localparam ECC_PATTERN = 8'b10000011;

	////////////////////////////////////////////////////////////
	reg c0_d2 = 0;
	reg c0_d3 = 0;

	reg c1_d0 = 0;
	reg c1_d1 = 0;
	reg c1_d2 = 0;
	reg c1_d3 = 0;

	reg c2_d0 = 0;
	reg c2_d1 = 0;
	reg c2_d2 = 0;
	reg c2_d3 = 0;

	////////////////////////////////////////////////////////////

	function [7:0] ECCcode; // Cycles the error code generator
		input [7:0] eccChksum;
		input dataBitToOutput;
		input passthroughData;
		begin
			ECCcode = (
				(eccChksum >> 1)
			 ^ (
					(
						(eccChksum[0] ^ dataBitToOutput) && passthroughData
					)
					? ECC_PATTERN : 8'b_0000_0000
				)
			);
		end
	endfunction

	task ECCu;
		output dataOutputChannel;
		inout [7:0] eccChksum;
		input dataBitToOutput;
		input passthroughData;
		begin
			dataOutputChannel <= passthroughData ? dataBitToOutput : eccChksum[0];
			eccChksum         <= ECCcode(eccChksum, dataBitToOutput, passthroughData);
		end
	endtask

	task ECC2u;
		output dataOutputChannel1;
		output dataOutputChannel2;
		inout [7:0] eccChksum;
		input dataBitToOutput1;
		input dataBitToOutput2;
		input passthroughData;
		begin
			dataOutputChannel1 <= passthroughData ? dataBitToOutput1 : eccChksum[0];
			dataOutputChannel2 <= passthroughData ? dataBitToOutput2 : (eccChksum[1] ^ (((eccChksum[0] ^ dataBitToOutput1) && passthroughData) ? 1'b1 : 1'b0));
			eccChksum          <= ECCcode(ECCcode(eccChksum, dataBitToOutput1, passthroughData), dataBitToOutput2, passthroughData);
		end
	endtask

	reg [7:0]  chkHeader = 0;
	reg [7:0]  chkPacket0 = 0;
	reg [7:0]  chkPacket1 = 0;
	reg [7:0]  chkPacket2 = 0;
	reg [7:0]  chkPacket3 = 0;
	reg [4:0]  dataOffset;

	task SendPacket;
		inout [31:0] header0;
		inout [31:0] header1;
		inout [55:0] subpacket0;
		inout [55:0] subpacket1;
		inout [55:0] subpacket2;
		inout [55:0] subpacket3;
		begin
			// channel 0
			ECCu(c0_d2, chkHeader, header0[0], dataOffset < 24 ? 1'b1 : 1'b0);
			c0_d3 <= header1[0];
			header0 <= header0[23:1];
			header1 <= header1[31:1];

			// channel 1 and 2
			ECC2u(c1_d0, c2_d0, chkPacket0, subpacket0[0], subpacket0[1], dataOffset < 28 ? 1'b1 : 1'b0);
			ECC2u(c1_d1, c2_d1, chkPacket1, subpacket1[0], subpacket1[1], dataOffset < 28 ? 1'b1 : 1'b0);
			ECC2u(c1_d2, c2_d2, chkPacket2, subpacket2[0], subpacket2[1], dataOffset < 28 ? 1'b1 : 1'b0);
			ECC2u(c1_d3, c2_d3, chkPacket3, subpacket3[0], subpacket3[1], dataOffset < 28 ? 1'b1 : 1'b0);
			subpacket0 <= subpacket0[55:2];
			subpacket1 <= subpacket1[55:2];
			subpacket2 <= subpacket2[55:2];
			subpacket3 <= subpacket3[55:2];

			dataOffset <= dataOffset+5'b1;
		end
	endtask
	//=======================================================
	//  REG/WIRE declarations
	//=======================================================

	wire [9:0] TMDS_red, TMDS_green, TMDS_blue;

	//=======================================================
	//  Structural coding
	//=======================================================

	reg [31:0] header [1:0];
	reg [63:0] subpacket [3:0];

	reg [31:0] audio_header [1:0];
	reg [63:0] audio_subpacket [3:0];

	reg [2:0]  audio_sample_counter = 0;
	reg [2:0]  audio_sample_counter_int = 0;
	// channel info counter
	// regen counter

	reg [7:0] channelStatusIdx = 0;
	reg [5:0] sendRegenCounter = 0;
	reg sendRegenPacket = 0;
	reg sendAudioPacket = 0;

	localparam header1_packet = 32'b11111111_11111111_11111111_11111110;
	localparam [191:0] channelStatus = 192'h_f2_00_00_40_04;

	initial begin
		header[0] = 32'h00000000;
		header[1] = 32'h00000000;

		subpacket[0] = 64'h0000000000000000;
		subpacket[1] = 64'h0000000000000000;
		subpacket[2] = 64'h0000000000000000;
		subpacket[3] = 64'h0000000000000000;

		audio_header[0] = 32'h00000000;
		audio_header[1] = 32'h00000000;
		
		audio_subpacket[0] = 64'h0000000000000000;
		audio_subpacket[1] = 64'h0000000000000000;
		audio_subpacket[2] = 64'h0000000000000000;
		audio_subpacket[3] = 64'h0000000000000000;
	end
	
	///////////////////////////////
	// ECC stuff
	///////////////////////////////
	always @(posedge pixclk_global) begin
		////////////////////////////////////////////////////////
		// reset audio sample count after packets are sent
		if (counterX == 816) begin
			audio_sample_counter <= 0;
		end
		// aquire audio packets
		if (!audio_rdempty /*&& counterX != 751 header prep pixel */ && (counterX < 751 || counterX > 816) /* audio data send period */) begin
			audio_rdreq <= 1'b1;
		end
		if (audio_rdempty) begin
			audio_rdreq <= 1'b0;
		end
		if (audio_rdreq) begin
			audio_sample_counter_int <= audio_sample_counter_int + 1'b1;
			if (audio_sample_counter_int == 0) begin
				// audio header
				audio_header[0] <= audio_header[0] | 24'b00000000_00000000_00000010;
			end else if (audio_sample_counter_int > 0) begin
				// audio packet
				audio_subpacket[audio_sample_counter] <= (
					  (
						  (audio_data[31:16] << 8) | (audio_data[15:0] << 32)
						| ( (^audio_data[31:16]) ? 56'h08000000000000 : 56'h0)
						| ( (^audio_data[15:0])  ? 56'h80000000000000 : 56'h0)
					  ) ^ (channelStatus[channelStatusIdx] ? 56'hCC000000000000 : 56'h0)
				);

				// set sample_present header
				audio_header[0][8 + audio_sample_counter] <= 1'b1;

				// set B.x sample header
				if (channelStatusIdx == 0) begin
					audio_header[0][20 + audio_sample_counter] <= 1'b1;
				end

				// channel status index counter
				if (channelStatusIdx < 191) begin
					channelStatusIdx <= channelStatusIdx + 1'b1;
				end else begin
					channelStatusIdx <= 0;
				end
				
				audio_sample_counter <= audio_sample_counter + 1'b1;
				sendAudioPacket <= 1'b1;
			end
		end else begin
			audio_sample_counter_int <= 0;
		end
		////////////////////////////////////////////////////////

		// prepare first package, once per line
		if (counterX == 751) begin
			// fixed
			header[1] <= header1_packet;
			audio_header[1] <= header1_packet;

			if (sendRegenPacket) begin
				// Audio Regen Package
				header[0] <= 24'h000001;
				subpacket[0] <= 56'h80180030750000;
				subpacket[1] <= 56'h80180030750000;
				subpacket[2] <= 56'h80180030750000;
				subpacket[3] <= 56'h80180030750000;
				sendRegenPacket <= 1'b0;
			end else begin
				if (!sendAudioPacket || !counterY[0]) begin
					// AVI Info Frame
					// populate data
					header[0] <= 24'h0D0282;
					//subpacket0 <= 56'h000002002a1033;
					//subpacket[0] <= 56'h0000020000006d;
					//subpacket[0] <= 56'h000002002b1032;
					subpacket[0] <= 56'h00000200291034;
					subpacket[1] <= 56'h00000000000000;
					subpacket[2] <= 56'h00000000000000;
					subpacket[3] <= 56'h00000000000000;
				end else begin
					// Audio Info Frame
					header[0] <= 24'h0A0184;
					subpacket[0] <= 56'h00000000000170;
					subpacket[1] <= 56'h00000000000000;
					subpacket[2] <= 56'h00000000000000;
					subpacket[3] <= 56'h00000000000000;
				end
			end
			
			// send regen packet every 49 audio packets
			if (sendRegenCounter + audio_sample_counter < 35/*orig: 48*/) begin
				sendRegenCounter <= sendRegenCounter + audio_sample_counter;
			end else begin
				sendRegenPacket <= 1'b1;
				// I don't know, if the packet sent exceeding regen period should shorten regen period
				//sendRegenCounter <= sendRegenCounter + audio_sample_counter - 6'd48;
				sendRegenCounter <= 1'b0;
			end
		end

		if (counterX >= 752 && counterX < 784) begin
			// first data package
			SendPacket(
				header[0],
				header[1],
				subpacket[0],
				subpacket[1],
				subpacket[2],
				subpacket[3]
			);
		end else if (counterX >= 784 && counterX < 816) begin
			// second data package (always audio or null packet)
			SendPacket(
				audio_header[0],
				audio_header[1],
				audio_subpacket[0],
				audio_subpacket[1],
				audio_subpacket[2],
				audio_subpacket[3]
			);
		end
	end

	/////////TMDS ENCODER

	defparam encode_blue.CHANNEL = "BLUE";
	encodez encode_blue
	(
		.clkin(pixclk_global) ,	// input  clkin_sig
		.rstin(reset) ,			// input  rstin_sig
		.din(blue[7:0]) ,			// input [7:0] vdin_sig
		.c0(hSync) ,				// input  c0_sig
		.c1(vSync) ,				// input  c1_sig
		.counterX(counterX),
		.counterY(counterY),
		.de(DrawArea) ,			// input  vde_sig
		.iDataD0(hSync), 			// data D0 is always hSync on channel 0
		.iDataD1(vSync), 			// data D1 is always vSync on channel 0
		.iDataD2(c0_d2),
		.iDataD3(c0_d3),
		.dout(TMDS_blue[9:0]) 	// output [9:0] dout_sig
	);

	defparam encode_green.CHANNEL = "GREEN";
	encodez encode_green
	(
		.clkin(pixclk_global) ,	// input  clkin_sig
		.rstin(reset) ,			// input  rstin_sig
		.din(green[7:0]) ,		// input [7:0] vdin_sig
		.c0(1'b0) ,					// input  c0_sig
		.c1(1'b0) ,					// input  c1_sig
		.counterX(counterX),
		.counterY(counterY),
		.de(DrawArea) ,			// input  vde_sig
		.iDataD0(c1_d0),
		.iDataD1(c1_d1),
		.iDataD2(c1_d2),
		.iDataD3(c1_d3),
		.dout(TMDS_green[9:0]) 	// output [9:0] dout_sig
	);

	defparam encode_red.CHANNEL = "RED";
	encodez encode_red
	(
		.clkin(pixclk_global),	// input  clkin_sig
		.rstin(reset) ,			// input  rstin_sig
		.din(red[7:0]) ,			// input [7:0] vdin_sig
		.c0(1'b0) ,					// input  c0_sig
		.c1(1'b0) ,					// input  c1_sig
		.counterX(counterX),
		.counterY(counterY),
		.de(DrawArea) ,			// input  vde_sig
		.iDataD0(c2_d0),
		.iDataD1(c2_d1),
		.iDataD2(c2_d2),
		.iDataD3(c2_d3),
		.dout(TMDS_red[9:0])		// output [9:0] dout_sig
	);

	//serializers
	wire [1:0]ddio_red;
	wire [1:0]ddio_green;
	wire [1:0]ddio_blue;
	simple_serializer ser_blue
	(
		.pclkx5(pixclkx5_global) ,				// input  pclkx5_sig
		.pclkx5_not(pixclkx5_not_global),	// input  pclkx5_not_sig
		.TMDS_red(TMDS_red),						// input [9:0] TMDS_red_sig
		.TMDS_green(TMDS_green),				// input [9:0] TMDS_green_sig
		.TMDS_blue(TMDS_blue),					// input [9:0] TMDS_blue_sig
		.ser_red(ddio_red),						// output [1:0] ser_red_sig
		.ser_green(ddio_green),					// output [1:0] ser_green_sig
		.ser_blue(ddio_blue) 					// output [1:0] ser_blue_sig
	);


	//DDDIO

	//wire red_component;
	DDIO red_com(
		.datain_h(ddio_red[0]),
		.datain_l(ddio_red[1]),
		.outclock(pixclkx5_global),
		.dataout(R)
	);

	DDIO green_com(
		.datain_h(ddio_green[0]),
		.datain_l(ddio_green[1]),
		.outclock(pixclkx5_global),
		.dataout(G)
	);

	DDIO blue_com(
		.datain_h(ddio_blue[0]),
		.datain_l(ddio_blue[1]),
		.outclock(pixclkx5_global),
		.dataout(B)
	);

	DDIO clk_com(
		.datain_h(1),
		.datain_l(0),
		.outclock(pixclk_global),
		.dataout(C)
	);

endmodule
