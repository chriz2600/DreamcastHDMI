// synopsys translate_off
`timescale 1 ps / 1 ps
// synopsys translate_on
module data_fifo (
	data,
	rdclk,
	rdreq,
	wrclk,
	wrreq,
	q,
	rdempty,
	wrfull);

    parameter WIDTH = 24;

	input	[WIDTH-1:0]  data;
	input	  rdclk;
	input	  rdreq;
	input	  wrclk;
	input	  wrreq;
	output	[WIDTH-1:0]  q;
	output	  rdempty;
	output	  wrfull;

	wire [WIDTH-1:0] sub_wire0;
	wire  sub_wire1;
	wire  sub_wire2;
	wire [WIDTH-1:0] q = sub_wire0[WIDTH-1:0];
	wire  rdempty = sub_wire1;
	wire  wrfull = sub_wire2;

	dcfifo	dcfifo_component (
				.data (data),
				.rdclk (rdclk),
				.rdreq (rdreq),
				.wrclk (wrclk),
				.wrreq (wrreq),
				.q (sub_wire0),
				.rdempty (sub_wire1),
				.wrfull (sub_wire2),
				.aclr (),
				.eccstatus (),
				.rdfull (),
				.rdusedw (),
				.wrempty (),
				.wrusedw ());
	defparam
		dcfifo_component.intended_device_family = "Cyclone 10 LP",
		dcfifo_component.lpm_numwords = 4,
		dcfifo_component.lpm_showahead = "OFF",
		dcfifo_component.lpm_type = "dcfifo",
		dcfifo_component.lpm_width = WIDTH,
		dcfifo_component.lpm_widthu = 2,
		dcfifo_component.overflow_checking = "ON",
		dcfifo_component.rdsync_delaypipe = 5,
		dcfifo_component.underflow_checking = "ON",
		dcfifo_component.use_eab = "OFF",
		dcfifo_component.wrsync_delaypipe = 5;


endmodule
