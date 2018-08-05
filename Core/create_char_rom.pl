#!/usr/bin/perl -w

my $x=0;

open(MIF_OUT, "> char_rom.v");

print MIF_OUT <<EOF
module char_rom (
    input [10:0] address,
    input clock,
    output [7:0] q
);

reg[7:0] q_reg;
reg[7:0] q_reg_2;

assign q = q_reg_2;

always @(posedge clock) begin
    case (address)
EOF
;

open(MIF_IN, "< char_rom.mif.in");
while(<MIF_IN>){
    chomp($_);
    $_ =~ s:^([0-9]{8};) -- (.*)$:$1 // $2:g;
    if ($_ =~ /^\s*--/) { 
        print MIF_OUT "        // " . $_ . "\n";
    } elsif ($_ =~ /^\s*$/) { 
        ; 
    } else { 
        print MIF_OUT sprintf("        %04d", $x) . ": q_reg <= 8'b" . $_ . "\n";
        $x++;
    }
}
close(MIF_IN);

print MIF_OUT <<EOF
    endcase
    q_reg_2 <= q_reg;
end
endmodule
EOF
;

close(MIF_OUT);