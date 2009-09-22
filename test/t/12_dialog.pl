#!/usr/bin/perl
use warnings;
use strict;
use Test::More;

my $dir = "../Common/Data/Dialogs/";
open  (IN, "ls $dir*.xml |");
my @files;
while (<IN>) {
	chomp;
	push @files, $_;
}
die "No files" if (!scalar(@files));

plan tests => scalar(@files);

foreach my $file (@files) {
	ok( system("xmllint $file > /dev/null") == 0, "Testing $file");
}

