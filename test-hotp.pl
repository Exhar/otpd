#!/usr/bin/perl
use Authen::HOTP qw(hotp);
#use strict;

%h2b = (0 => "0000", 1 => "0001", 2 => "0010", 3 => "0011",
4 => "0100", 5 => "0101", 6 => "0110", 7 => "0111",
8 => "1000", 9 => "1001", a => "1010", b => "1011",
c => "1100", d => "1101", e => "1110", f => "1111",
);

my $secret="44D060008BF440A2F9FF588AAD537F78B820F200";
my $counter=1;
my $digits=6;
#my $pass = hotp($secret, $counter, $digits);
#print "$pass\n";

($secret = $secret) =~ s/(.)/$h2b{lc $1}/g
        if $secret =~ /^[a-fA-F0-9]{32,}$/;
#$secret = join("", map chr(hex), $secret =~ /(..)/g)

print $secret




