#include "pipe.h"

struct pipe_context *pipe_vm_context;

void set_pipe(struct pipe_context *pipe_context) {
	pipe_vm_context = pipe_context;
}

int pipealloc(struct file *f0, struct file *f1)
{
	struct pipe *pi;
	pi = 0;
	if ((pi = (struct pipe *)kalloc()) == 0)
		goto bad;
	pi->readopen = 1;
	pi->writeopen = 1;
	pi->nwrite = 0;
	pi->nread = 0;
	f0->type = FD_PIPE;
	f0->readable = 1;
	f0->writable = 0;
	f0->pipe = pi;
	f1->type = FD_PIPE;
	f1->readable = 0;
	f1->writable = 1;
	f1->pipe = pi;
	return 0;
bad:
	if (pi)
		kfree((char *)pi);
	return -1;
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
		kfree((char *)pi);
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
			(pipe_vm_context->yield)();
		} else {
			size = MIN(MIN(n - w,
				       pi->nread + PIPESIZE - pi->nwrite),
				   PIPESIZE - (pi->nwrite % PIPESIZE));
			if (copyin((pipe_vm_context->get_curr_pagetable)(),
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
			(pipe_vm_context->yield)();
		else
			return -1;
	}
	while (r < n && size != 0) { // DOC: piperead-copy
		if (pi->nread == pi->nwrite)
			break;
		size = MIN(MIN(n - r, pi->nwrite - pi->nread),
			   PIPESIZE - (pi->nread % PIPESIZE));
		if (copyout((pipe_vm_context->get_curr_pagetable)(), addr + r,
			    &pi->data[pi->nread % PIPESIZE], size) < 0) {
			panic("copyout");
		}
		pi->nread += size;
		r += size;
	}
	return r;
}
