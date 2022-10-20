#include "pipe.h"

struct pipe_context *pipe_os_context;

void set_pipe(struct pipe_context *pipe_context) {
	pipe_os_context = pipe_context;
}

void* pipeopen()
{
	struct pipe *pi = (struct pipe*)(pipe_os_context->kalloc)();
	if (pi == 0)
		return 0;
	pi->readopen = 1;
	pi->writeopen = 1;
	pi->nwrite = 0;
	pi->nread = 0;
	return pi;
}

void pipeclose(void *_pi, int writable)
{
	struct pipe *pi = (struct pipe*)(_pi);
	if (writable) {
		pi->writeopen = 0;
	} else {
		pi->readopen = 0;
	}
	if (pi->readopen == 0 && pi->writeopen == 0) {
		(pipe_os_context->kfree)((char *)pi);
	}
}

int pipewrite(void *_pi, uint64 addr, int n)
{
	struct pipe *pi = (struct pipe*)(_pi);
	int w = 0;
	uint64 size;
	if (n <= 0) {
		panic("invalid read num");
	}
	while (w < n) {
		if (pi->readopen == 0) {
			return -1;
		}
		if (pi->nwrite == pi->nread + PIPESIZE) { // DOC: pipewrite-full
			(pipe_os_context->yield)();
		} else {
			size = MIN(MIN(n - w,
				       pi->nread + PIPESIZE - pi->nwrite),
				   PIPESIZE - (pi->nwrite % PIPESIZE));
			if ((pipe_os_context->copyin)((pipe_os_context->get_curr_pagetable)(),
				   &pi->data[pi->nwrite % PIPESIZE], addr + w,
				   size) < 0) {
				panic("copyin");
			}
			pi->nwrite += size;
			w += size;
		}
	}
	return w;
}

int piperead(void *_pi, uint64 addr, int n)
{
	struct pipe *pi = (struct pipe*)(_pi);
	int r = 0;
	uint64 size = -1;
	if (n <= 0) {
		panic("invalid read num");
	}
	while (pi->nread == pi->nwrite) {
		if (pi->writeopen)
			(pipe_os_context->yield)();
		else
			return -1;
	}
	while (r < n && size != 0) { // DOC: piperead-copy
		if (pi->nread == pi->nwrite)
			break;
		size = MIN(MIN(n - r, pi->nwrite - pi->nread),
			   PIPESIZE - (pi->nread % PIPESIZE));
		if ((pipe_os_context->copyout)((pipe_os_context->get_curr_pagetable)(), addr + r,
			    &pi->data[pi->nread % PIPESIZE], size) < 0) {
			panic("copyout");
		}
		pi->nread += size;
		r += size;
	}
	return r;
}
