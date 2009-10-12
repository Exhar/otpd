#!/usr/bin/perl

use Data::Translate;
$data=new Translate;

$str = $ARGV[0];
$length = length($str);
$current=0;
do {
$char = substr($str,$current,1);
($s,$hh)=$data->a2h($char);
$current++;
print "$hh";

} until ($current > $length ) ;
print "\n";
