#!/usr/bin/perl
use Authen::HOTP qw(hotp);
use strict;

#my $secret="44D060008BF440A2F9FF588AAD537F78B820F200";
#my $secret="12345678901234567890";
my $secret="f34703181f5588868e20d01bc3cf121d58bd7804";
my $counter=$ARGV[1];
my $digits=$ARGV[0];
my $pass = hotp($secret, $counter, $digits);
print "$pass\n";


