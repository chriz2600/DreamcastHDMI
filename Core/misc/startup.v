`include "config.inc"

module startup(
    input clock,
    input nreset,
    output reg ready,
    input [31:0] startup_delay
);
    reg startup_done = 0;
    reg [31:0] counter;

    initial begin
        ready <= 0;
        counter <= 0;
    end

    always @ (posedge clock or negedge nreset)
    begin
        if (~nreset) begin
            ready <= 0;
            counter <= 0;
        end else begin
            counter <= #1 counter + 1;
            
            if (startup_done || counter == startup_delay) begin
                ready <= 1;
                startup_done <= 1;
            end
        end
    end
    
endmodule