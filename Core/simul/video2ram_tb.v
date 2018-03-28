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
  wire [`RAM_ADDRESS_BITS-1:0] wraddr;
  wire wren;
  wire wrclock;
  wire starttriggerIn;

  wire [`RAM_ADDRESS_BITS-1:0] rdaddr;
  wire starttriggerOut;

  wire HSYNC;
  wire VSYNC;
  wire DE;
  wire CLOCK;
  
  wire [7:0] RED;
  wire [7:0] GREEN;
  wire [7:0] BLUE;

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
    .red(RED),
    .green(GREEN),
    .blue(BLUE)
  );


  always #250 clkIn = ~clkIn;
  always #91 clkOut = ~clkOut;

  always @(posedge clkIn) begin
    if (counterX < 858) begin
      counterX <= counterX + 1;
    end else begin
      counterX <= 0;
      if (counterY < 525) begin
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
      $monitor("%0dx%0d %0d %0dx%0d %0d", counterX, counterY, wraddr, ram2video.counterX_reg, ram2video.counterY_reg, rdaddr);
      //$monitor("x:%0d y:%0d ray:%0d", counterX, counterY, video2ram.ram_addrY_reg);
      wait (starttriggerOut) begin
        $display("TRIGGER");
      end

      wait ((counterY >= 480 && ram2video.counterX_reg >= 960) || (1 && `__IsCaptureTime(counterX, counterY) && `IsDrawAreaVGA(ram2video.counterX_reg, ram2video.counterY_reg) && wraddr == rdaddr)) begin
        $display("stop: y:%0d ay:%0d ay2:%0d", counterY, wraddr, rdaddr);
        $finish;
      end

      // wait (counterY == 34) begin
      //   $display("stop %0d", ram2video.counterX_reg);
      //   $finish;
      // end


      // wait (counterY == 480) begin
      //   $display("stop %0d", ram2video.counterX_reg);
      //   $finish;
      // end
    end
endmodule
