#!/usr/bin/perl -w

my $x=0;

if (!defined($ARGV[0]) or $ARGV[0] =~ /^\s*$/ or !defined($ARGV[1]) or $ARGV[1] =~ /^\s*$/) {
    print "\n  usage: create_from_mif <input_mif_file> <output_verilog_fragment_file>\n\n";
    exit 1;
}

open(MIF_OUT, "> " . $ARGV[1]);

print MIF_OUT <<EOF
    case (address)
EOF
;

my $start = 0;

open(MIF_IN, "< " . $ARGV[0]);
while(<MIF_IN>){
    chomp($_);

    if ($start && $_ =~ /^END;/) {
        $start = 0;
    }

    if ($start) {
        if ($_ =~ /([0-9]+)\s*:\s*([0-9])\s*;(.*)$/) {
            print MIF_OUT sprintf("        %d", $1) . ": q_reg <= 1'b" . $2 . "; // " . $3 . "\n";
        }
    }

    if ($_ =~ /^CONTENT BEGIN/) {
        $start = 1;
    }
}
close(MIF_IN);

print MIF_OUT <<EOF
    endcase
EOF
;

close(MIF_OUT);