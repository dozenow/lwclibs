syscall::lwccreate:entry {
	self->ts = timestamp;
}
syscall::lwccreate:return /self->ts/ {
	@[execname] = quantize(timestamp - self->ts);
	self->ts = 0;
}

