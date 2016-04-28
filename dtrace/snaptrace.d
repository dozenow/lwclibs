syscall:freebsd:lwccreate:entry {
	star4 = 4; /*(int) copyin(arg4, 4);*/
	printf("lwccreate(0x%lx, %d, 0x%lx, 0x%lx, 0x%lx(%d), 0x%x)", arg0, arg1, arg2, arg3, arg4, star4, arg5);
}

syscall::lwccreate:return {
	printf("lwccreate rv = %d", arg1);
}

syscall:freebsd:lwcsuspendswitch:entry {
	star5 = 5; /*(int) copyin(arg5, 4);*/
	printf("lwcsuspendswitch(%d, 0x%lx, %d, 0x%lx, 0x%lx, 0x%lx(%d))", arg0, arg1, arg2, arg3, arg4, arg5, star5);
}

syscall:freebsd:lwcdiscardswitch:entry {
	star5 = 5; /*(int) copyin(arg5, 4);*/
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
	printf("lwcsuspendswitch rv = %d", arg1);
}

lwc:kern_lwc:suspendswitch:syslwc {
	printf("suspendswitch from 0x%lx to 0x%lx", arg0, arg1);
}

lwc:kern_lwc:discardswitch:syslwc {
	printf("discardswitch from 0x%lx to 0x%lx", arg0, arg1);
}

lwc:kern_lwc:alloc:syslwc {
	printf("allocated lwc 0x%lx", arg0);
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

/*fbt:kernel:snap_fd:entry {
	printf("Entering snap_fd, have s of ");
	self->s = args[1];
	print(*args[1]);
}

fbt:kernel:snap_fd:return {
	printf("exiting snap_fd, have s of ");
	print(*(self->s));
}
*/


lwc:kern_lwc:forkfd:syslwc {
	printf("current fd forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:sharefd:syslwc {
	/* print(*args[0]); show all of the struct, */
	printf("current fd shared with lwc 0x%lx, refcnt=%d", arg0, args[0]->se_fdesc->fd_refcnt);
}

lwc:kern_lwc:forkcred:syslwc {
	printf("current cred forked into lwc 0x%lx", arg0);
}

lwc:kern_lwc:sharecred:syslwc {
	/* print(*args[0]); show all of the struct, */
	printf("current cred shared with lwc 0x%lx, refcnt=%d", arg0, args[0]->se_cred->cr_ref);
}
