syscall:freebsd:lwccreate:entry {
	star4 = arg4 ? (int) copyin(arg4, 4) : -1;
	printf("lwccreate(0x%lx, %d, 0x%lx, 0x%lx, 0x%lx(%d), 0x%x)", arg0, arg1, arg2, arg3, arg4, star4, arg5);
}

syscall::lwccreate:return {
	printf("lwccreate rv = %d", arg1);
}

syscall:freebsd:lwcsuspendswitch:entry {
	star5 = arg5 ? (int) copyin(arg5, 4) : -1;
	printf("lwcsuspendswitch(%d, 0x%lx, %d, 0x%lx, 0x%lx, 0x%lx(%d))", arg0, arg1, arg2, arg3, arg4, arg5, star5);
}

syscall:freebsd:lwcdiscardswitch:entry {
	printf("lwcdiscardswitch(%d, 0x%lx, %d)", arg0, arg1, arg2);
}

syscall::lwcoverlay:entry {
	printf("lwcoverlay(%d, 0x%lx, %d)", arg0, arg1, arg2);
}

syscall::lwcoverlay:return {
	printf("lwcoverlay rv = %d", arg1);
}

syscall::lwcsuspendswitch:return {
	printf("lwcsuspendswitch rv = %d", arg1);
}


syscall::lwcdiscardswitch:return {
	printf("lwcdiscardswitch rv = %d", arg1);
}

lwc:kern_lwc:suspendswitch:syslwc {
	printf("suspendswitch from 0x%lx to 0x%lx", arg0, arg1);
}

lwc:kern_lwc:discardswitch:syslwc {
	printf("discardswitch from 0x%lx to 0x%lx", arg0, arg1);
}

lwc:kern_lwc:alloc:syslwc {
	printf("allocated lwc 0x%lx, pid=%d", arg0, pid);
}

lwc:kern_lwc:createdproto:syslwc {
	printf("createproto 0x%lx", arg0);
}

lwc:kern_lwc:overlay:syslwc {
	/* you can deref and chase these guys */
	printf("createproto snap 0x%lx requested 0x%lx permitted 0x%lx", arg0, arg1, arg2);
}

lwc:kern_lwc:copy:syslwc {
	printf("copied lwc 0x%lx", arg0);
}

lwc:kern_lwc:free:snrelease {
	printf("freeing lwc 0x%lx", arg0);
}

lwc:kern_lwc:forkcpu:syslwc {
	printf("current cpu status copied into into lwc 0x%lx", arg0);
}

lwc:kern_lwc:forkvm:syslwc {
	printf("current vm forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:sharevm:syslwc {
	printf("current shared forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:forkfd:syslwc {
	printf("current fd forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:sharefd:syslwc {
	/* print(*args[0]); show all of the struct, */
	printf("current fd shared with lwc 0x%lx", arg0);
}

lwc:kern_lwc:forkcred:syslwc {
	printf("current cred forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:sharecred:syslwc {
	/* print(*args[0]); show all of the struct, */
	printf("current cred shared with lwc 0x%lx", arg0);
}
