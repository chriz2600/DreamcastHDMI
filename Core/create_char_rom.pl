#!/usr/bin/perl -w

my $x=0;

open(MIF_OUT, "> char_rom.mif");

print MIF_OUT <<EOF
-- character ROM
--   - 8-by-16 (8-by-2^4) font
--   - 128 (2^7) characters
--   - ROM size: 512-by-8 (2^11-by-8) bits
--               16K bits
-- created by create_char_rom.pl from char_rom.mif.in

DEPTH = 2048;
WIDTH = 8;
ADDRESS_RADIX = DEC;
DATA_RADIX = BIN;

CONTENT
BEGIN
EOF
;

open(MIF_IN, "< char_rom.mif.in");
while(<MIF_IN>){
    chomp($_);
    if ($_ =~ /^\s*--/) { 
        print MIF_OUT $_ . "\n"; 
    } elsif ($_ =~ /^\s*$/) { 
        ; 
    } else { 
        print MIF_OUT sprintf("%04d", $x) . " : " . $_ . "\n";
        $x++;
    }
}
close(MIF_IN);

print MIF_OUT <<EOF
END;
EOF
;

close(MIF_OUT);