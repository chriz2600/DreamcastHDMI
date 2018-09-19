#!/usr/bin/perl -w

use strict;
use File::Basename;

chdir dirname($0);

sub dec2bin {
    my $str = unpack("B32", pack("N", shift));
    $str =~ s/^0+(?=\d)(........)$/$1/;
    return $str;
}

open(OUT, "> text_ram.mif");
print OUT <<EOF
-- created by create_text_ram.pl from text_ram.txt

DEPTH = 1024;
WIDTH = 8;
ADDRESS_RADIX = DEC;
DATA_RADIX = BIN;

CONTENT
BEGIN
EOF
;

my $version = $ENV{'CI_COMMIT_REF_NAME'};

unless (defined($version)) {
    $version = "";
}

if ($version eq "master") {
    $version = $ENV{'CI_COMMIT_SHA'};
}

if ($version eq "") {
    $version = `date +"%Y%m%d%H%M%S"`
}

chomp($version);
$version = sprintf("%13s", $version);

my $x = 0;
my $data="";
open(TXT, "< text_ram.txt");
while (<TXT>) {
    chomp($_);
    $_ =~ s/__FW_VERSION__/$version/g;
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
