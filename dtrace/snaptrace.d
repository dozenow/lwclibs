syscall::snap:entry {
	printf("snap(%d, %d, %d)", arg0, arg1, arg2);
	self->ts = timestamp;
}

snap:kern_snap:jump:syssnap {
	printf("jump from 0x%lx to 0x%lx", arg0, arg1);
}

snap:kern_snap:alloc:syssnap {
	printf("allocated snap 0x%lx", arg0);
}

snap:kern_snap:copy:syssnap {
	printf("copied snap 0x%lx", arg0);
}

snap:kern_snap:update:syssnap {
	printf("updating snap 0x%lx refcnt=%d", arg0, args[0]->se_refcnt);
}

snap:kern_snap:free:snrelease {
	printf("freeing snap 0x%lx", arg0);
}

snap:kern_snap:forkvm:syssnap {
	printf("current vm forked into snap 0x%lx", arg0);
}

snap:kern_snap:sharevm:syssnap {
	/* print(*args[0]); show all of the struct, */
	printf("current vm shared with snap 0x%lx, refcnt=%d", arg0, args[0]->se_vmspace->vm_refcnt); 
}

snap:kern_snap:forkfd:syssnap {
	printf("current fd forked into snap 0x%lx", arg0);
}

snap:kern_snap:sharefd:syssnap {
	/* print(*args[0]); show all of the struct, */
	printf("current fd shared with snap 0x%lx, refcnt=%d", arg0, args[0]->se_fdesc->fd_refcnt);
}

snap:kern_snap:forkcred:syssnap {
	printf("current cred forked into snap 0x%lx", arg0);
}

snap:kern_snap:sharecred:syssnap {
	/* print(*args[0]); show all of the struct, */
	printf("current cred shared with snap 0x%lx, refcnt=%d", arg0, args[0]->se_cred->cr_ref);
}



syscall::snap:return {
	printf("snap rv = %d", arg1);
}
