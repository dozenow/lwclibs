#pragma D option quiet
/* syscall::lwccreate:entry { */
/* 	self->create_ts = timestamp; */
/* } */

/* syscall::lwccreate:return { */
/* 	@res["create_all"] = sum((timestamp - self->create_ts) / 1000); */
/* 	self->create_ts = 0; */
/* } */

/* fbt:kernel:vmspace_lwc_fork:entry { */
/* 	self->vmspace_fork = timestamp; */
/* } */

/* fbt:kernel:vmspace_lwc_fork:return { */
/* 	@res["vmfork_elapsed"] = sum((timestamp - self->vmspace_fork) / 1000 ); */
/* 	self->vmspace_fork = 0; */
/* } */


syscall::lwcclose:entry {
	close_ts = timestamp;
}

syscall::lwcclose:return {
	@res["close_elapsed"] = sum((timestamp - close_ts) / 1000);
	@counts["lwcclose"] = count();
	close_ts = 0;
}

lwc:kern_lwc:free:syslwc /args[0] == NULL/ {
	@counts["nofreed"] = count();
}

lwc:kern_lwc:free:syslwc /args[0] != NULL/ {
	@counts["freed"] = count();
}




fbt:kernel:vmspace_free:entry {
	vmfree_ts = timestamp;

}

fbt:kernel:vmspace_free:return {
	printf("vmspace_free return\n");
	@res["vmfree_elapsed"] = sum((timestamp - vmfree_ts) / 1000);
	@counts["vmfree"] = count();	
	vmfree_ts = 0;
}

fbt:kernel:snrelease:entry /close_ts/ {
	release_ts = timestamp;
}

fbt:kernel:snrelease:return /close_ts/ {
	@res["release_elapsed"] = sum((timestamp - release_ts) / 1000);
	@counts["snrelease"] = count();	
	release_ts = 0;
}

fbt:kernel:get_target:entry /close_ts/ {
	target_ts = timestamp;
}

fbt:kernel:get_target:return /close_ts/ {
	@res["get_target_elapsed"] = sum((timestamp - target_ts) / 1000);
	target_ts = 0;
	@counts["get_target"] = count();	
}

fbt:kernel:kern_close:entry /close_ts/ {
	kclose_ts = timestamp;
}

fbt:kernel:kern_close:return /kclose_ts/ {
	printf("kern_close return\n");

	@res["kern_close_elapsed"] = sum((timestamp - kclose_ts) / 1000);
	@counts["kern_close"] = count();	
	kclose_ts = 0;
}

lwc:kern_lwc:close:syslwc /args[0]/ {
	close_ts = timestamp;
	@counts["official"] = count();
}

lwc:kern_lwc:close:syslwc /args[0] == NULL / {
	@counts["null"] = count();
}


tick-1sec {
	printf("Timing:\n");
	printa("%40s %10@d\n", @res);
	printf("Counts:\n");
 	printa("%40s %10@d\n", @counts);
	/*printa("%40s %10@d\n", @res); */
	clear(@counts);
	clear(@res);
}

BEGIN {
	printf("%40s | %s\n", "key", "nanoseconds per second");
}
