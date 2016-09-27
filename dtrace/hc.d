#!/usr/sbin/dtrace -s

#pragma D option stackframes=100
#pragma D option ustackframes=100
#pragma D option defaultargs
#pragma D option aggsize=8m
#pragma D option bufsize=16m
#pragma D option dynvarsize=16m
   /* #pragma D option aggrate=0 */
   /*#pragma D option cleanrate=50Hz */

profile:::profile-999
/execname == "switcher_pthread"/
{
	@[stack(), ustack(100,100), 1] = sum(1000);
}

sched:::off-cpu
/execname == "switcher_pthread"/
{
	self->start = timestamp;
}

sched:::on-cpu
/execname == "switcher_pthread" && (this->start = self->start)/
{
	this->delta = (timestamp - this->start) / 1000;
	@[stack(), ustack(100,100), 0] = sum(this->delta);
	self->start = 0;
}


syscall::nanosleep:entry /execname == "switcher_pthread"/ {
	normalize(@, 1000);
	printa("%k %k oncpu:%d ms:%@d\n", @);
	trunc(@);
	exit(0);
}