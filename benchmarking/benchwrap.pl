#!/usr/bin/env perl

use strict;
use warnings;

use Data::Dumper;

BEGIN {
	$ENV{IPCRUN3PROFILE} = 2;
}


use Cwd qw(abs_path chdir getcwd);
use autodie qw(:all);
use Getopt::Long;
use POSIX qw(strftime);
use File::Tee qw(tee);
use File::Basename;
use IPC::Run3 qw(run3);

my $verbose = 1;
my $opts = '';
my $help = 0;
my $save_results = 1;
my $cmd = '';
my $dir = getcwd();

GetOptions('verbose!' => \$verbose, pts => \$opts, help => \$help, 'opts=s' => \$opts,
           'save_results!' => \$save_results, 'cmd=s' => \$cmd, 'dir=s' => \$dir);


# Example of use:
#./benchwrap.pl --cmd=/opt/apache2/bin/ab --opts='-n 1000 -c 4 http://thor23:8182/' --noverbose --save_results --dir="/tmp/"

# Run setup.pl to install any dependencies

sub print_usage {
	print qq{
	Usage: benchwrap.pl [options] 
		--verbose			Be verbose (default)
		--noverbose			Limit output
		--opts=s				A list of options to pass to the program to run as a test
		--help				Show this usage menu
		--save_results=1	Whether to save results, negateable with --nosave_results
		--cmd					The command to run
		--dir=cwd			The directory to store the results in
};
}

if ($help || !$cmd) {
	print_usage();
	exit(0);
}


die "You don't want to see the output or save it. No." if (!$verbose && !$save_results);

my $ident = `uname -i`;
chomp($ident);


$cmd =~ s{^~/}{$ENV{HOME}/}g;
$cmd = abs_path($cmd);
die "Command not found\n" unless $cmd;
chdir($dir) or die "Could not change directory: $?\n";



my $cmd_dir = dirname($cmd);
my $commit_line = `git -C $cmd_dir log --format='%h %ci %s %ai' -1 2>/dev/null`;

my ($out_fh,$err_fh);



if ($verbose) {
	$out_fh = \*STDOUT;
	$err_fh = \*STDERR;
}

if ($save_results) {
	printf("Saving results in %s\n", $dir);
	my $bname = basename($cmd);
	my $date = strftime('%Y%m%dZ%H%M%S', gmtime());
	my $out_file = "$date.$bname.out";
	my $err_file = "$date.$bname.err";
	die "Output file $out_file already exists" if (-f $out_file);
	die "Error file $err_file already exists" if (-f $err_file);
	if (!$verbose) {
		open($out_fh, '>', $out_file) or die "Could not open output file: $!";
		open($err_fh, '>', $err_file) or die "Could not open output file: $!";;
	} else {
		tee STDOUT, '>', $out_file or die "Could not open output file: $!";
		tee STDERR, '>', $err_file or die "Could not open output file: $!";;
	}
}

printf($err_fh "WARNING: Using $ident instead of evaluation kernel config\n\n") unless $ident eq 'SNAP_EVAL';

printf($out_fh "Beginning test of %s at %s\n", $cmd, strftime('%F %T %Z (%s)', localtime()));

printf($out_fh "CMD: %s %s\n", $cmd, $opts);
printf($out_fh "CMD COMMIT: %s", $commit_line ? $commit_line : "Command not under version control\n");
printf($out_fh "KERNEL: %s", `uname -a`);

my @c = ($cmd, split/ /,$opts);
eval {
	run3(\@c, undef, $out_fh, $err_fh);
};
if ($@) {
	printf($out_fh "Could not run test at %s : %s\n", strftime('%F %T %Z (%s)', localtime()), $@);
} else {
	printf($out_fh "Finished test at %s\n", strftime('%F %T %Z (%s)', localtime()));
}
