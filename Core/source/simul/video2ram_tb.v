`include "../config.inc"
`include "../video2ram.v"
`include "../edge_detect.v"
`include "../Flag_CrossDomain.v"
`include "../ram2video.v"

module video2ram_tb;


  reg clkIn = 0;
  reg clkOut = 0;
  reg [11:0] counterX;
  reg [11:0] counterY;

  wire [23:0] wrdata;
  wire [23:0] rddata;
  wire [14:0] wraddr;
  wire wren;
  wire wrclock;
  wire starttriggerIn;

  wire [14:0] rdaddr;
  wire starttriggerOut;

  wire HSYNC;
  wire VSYNC;
  wire DE;
  wire CLOCK;
  
  wire [23:0] video_out;

  video2ram video2ram(
    .clock(clkIn),
    .R(8'd0),
    .G(8'd0),
    .B(8'd0),
    .counterX(counterX),
    .counterY(counterY),
    .line_doubler(1'b0),
    .wrdata(wrdata),
    .wraddr(wraddr),
    .wren(wren),
    .wrclock(wrclock),
    .starttrigger(starttriggerIn)
  );

  Flag_CrossDomain trigger(
    .clkA(clkIn),
    .FlagIn_clkA(starttriggerIn),
    .clkB(clkOut),
    .FlagOut_clkB(starttriggerOut)
  );

  ram2video ram2video(
    .starttrigger(starttriggerOut),
    .clock(clkOut),
    .reset(1'b1),
    .line_doubler(1'b0),
    .add_line(1'b0),
    .rddata(rddata),
    .hsync(HSYNC),
    .vsync(VSYNC),
    .DrawArea(DE),
    .videoClock(CLOCK),
    .rdaddr(rdaddr),
    .video_out(video_out)
  );

`ifdef _1080p_
  initial $display("1080p");
  always #250 clkIn = ~clkIn;
  always #91 clkOut = ~clkOut;
`endif

`ifdef _960p_
  initial $display("960p");
  always #2000 clkIn = ~clkIn;
  always #1001 clkOut = ~clkOut;
`endif

`ifdef _VGA_
  initial $display("VGA");
  always #400 clkIn = ~clkIn;
  always #429 clkOut = ~clkOut;
`endif

  always @(posedge clkIn) begin
    if (counterX < 858 - 1) begin
      counterX <= counterX + 1;
    end else begin
      counterX <= 0;
      if (counterY < 525 - 1) begin
        //$display("y:%0d ay:%0d ay2:%0d - %0d %0d", counterY, video2ram.ram_addrY_reg, ram2video.ram_addrY_reg, (video2ram.ram_addrY_reg == ram2video.ram_addrY_reg), rdaddr);
        counterY <= counterY + 1;
      end else begin
        counterY <= 0;
      end
    end
  end

  `define __IsVerticalCaptureTime(y) ( \
      video2ram.line_doubler \
          ? (y < 240 || (y > 262 && y < video2ram.V_CAPTURE_END)) \
          : (y >= video2ram.V_CAPTURE_START && y < video2ram.V_CAPTURE_END) \
  )
  `define __IsCaptureTime(x,y) ( \
      `__IsVerticalCaptureTime(y) && x >= video2ram.H_CAPTURE_START && x < video2ram.H_CAPTURE_END \
  )

  initial 
    begin
      
      counterX <= 0;
      counterY <= 0;
      $monitor("%0d - %0d: %0dx%0d %0d(%0d) %0dx%0d %0d", $time, starttriggerOut, counterX, counterY, wraddr, wren, ram2video.counterX_reg, ram2video.counterY_reg, rdaddr);

      wait (starttriggerOut) begin
        $display("TRIGGER");
      end

      wait ((counterY >= 480 && ram2video.counterY_reg >= `VERTICAL_LINES_VISIBLE - `VERTICAL_OFFSET) || (wren && `IsDrawAreaVGA(ram2video.counterX_reg, ram2video.counterY_reg) && wraddr == rdaddr)) begin
        $display("stop: y:%0d ay:%0d ay2:%0d", counterY, wraddr, rdaddr);
        $finish;
      end
    end
endmodule
