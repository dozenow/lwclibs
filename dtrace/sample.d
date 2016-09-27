
profile-199 /execname == "switcher_pthr_ipc"/ {
	@[stack(100), ustack(100, 4096)] = count();
}

syscall::nanosleep:entry /execname == "switcher_pthr_ipc"/ {
	exit(0);
}

