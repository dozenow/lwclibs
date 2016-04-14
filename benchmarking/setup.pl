#!env perl

use strict;
use warnings;

use Data::Dumper;

my @pkg_needed = qw(
	                   p5-IPC-System-Simple
	                   p5-File-Tee
	                   p5-IPC-Run3
                  );

for (@pkg_needed) {
	system("sudo pkg install $_");
}
