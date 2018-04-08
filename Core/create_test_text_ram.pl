#!/usr/bin/perl -w

sub dec2bin {
    my $str = unpack("B32", pack("N", shift));
    $str =~ s/^0+(?=\d)(........)$/$1/;
    return $str;
}

open(OUT, "> text_ram.mif");
print OUT <<EOF
-- created by create_test_text_ram.pl from text_ram.txt

DEPTH = 1024;
WIDTH = 8;
ADDRESS_RADIX = DEC;
DATA_RADIX = BIN;

CONTENT
BEGIN
EOF
;

my $x=0;
my $data="";
open(TXT, "< text_ram.txt");
while (<TXT>) {
    chomp($_);
    $data .= $_;
}

for my $c (split //, $data) {
    print OUT sprintf("%04d", $x) . " : " . dec2bin(ord($c)) . "; -- $c\n";
    $x++;
}

while ($x < 1024) {
    print OUT sprintf("%04d", $x) . " : 00000000; -- padding\n";
    $x++;
}
close(TXT);

print OUT <<EOF
END;
EOF
;

close(OUT);