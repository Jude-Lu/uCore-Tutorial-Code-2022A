#ifndef PIPE_H
#define PIPE_H

#include "../utils/defs.h"
#include "../utils/riscv.h"

#define PIPESIZE (512)

//a struct for pipe
struct pipe {
	char data[PIPESIZE];
	uint nread; // number of bytes read
	uint nwrite; // number of bytes written
	int readopen; // read fd is still open
	int writeopen; // write fd is still open
};

struct file;

int pipealloc(struct file *f0, struct file *f1);
void pipeclose(struct pipe *pi, int writable);
int pipewrite(struct pipe *pi, uint64 addr, int n);
int piperead(struct pipe *pi, uint64 addr, int n);

struct pipe_context
{
	pagetable_t (*get_curr_pagetable)();
	void (*yield)();
};

void set_pipe(struct pipe_context *pipe_context);

#endif // PIPE_H