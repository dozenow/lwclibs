#!/usr/sbin/dtrace -s

#pragma D option stackframes=100
#pragma D option ustackframes=100
#pragma D option defaultargs
#pragma D option aggsize=8m
#pragma D option bufsize=16m
#pragma D option dynvarsize=16m

profile-499 /execname == "stress2"/ {
	@[stack(100), ustack(100, 4096)] = sum(500);
}

syscall::nanosleep:entry /execname == "stress2"/ {
	exit(0);
}

