#!/usr/sbin/dtrace -s

#pragma D option stackframes=100
#pragma D option ustackframes=100
#pragma D option defaultargs
#pragma D option aggsize=8m
#pragma D option bufsize=16m
#pragma D option dynvarsize=16m
   /* #pragma D option aggrate=0 */
   /*#pragma D option cleanrate=50Hz */


profile:::profile-499
/pid == $1/
{
	@[stack(), ustack(100,100), 1] = sum(500);
}

sched:::off-cpu
/pid == $1/
{
	self->start = timestamp;
}

sched:::on-cpu
/(this->start = self->start) && pid == $1/
{
	this->delta = (timestamp - this->start) / 1000;
	@[stack(), ustack(100,100), 0] = sum(this->delta);
	self->start = 0;
}


END {
	normalize(@, 1000);
	printa("%k %k oncpu:%d ms:%@d\n", @);
}
