syscall::snap:entry {
	self->ts = timestamp;
}
syscall::snap:return /self->ts/ {
	@[execname] = quantize(timestamp - self->ts);
	self->ts = 0;
}

