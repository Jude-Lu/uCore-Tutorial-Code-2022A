#ifndef PIPE_H
#define PIPE_H

#include "defs.h"
#include "log.h"

#define PIPESIZE (512)

//a struct for pipe
struct pipe {
	char data[PIPESIZE];
	uint nread; ///< number of bytes read
	uint nwrite; ///< number of bytes written
	int readopen; ///< read fd is still open
	int writeopen; ///< write fd is still open
};

void* pipeopen();
void pipeclose(void *_pi, int writable);
int pipewrite(void *_pi, uint64 addr, int n);
int piperead(void *_pi, uint64 addr, int n);

struct pipe_context
{
	pagetable_t (*get_curr_pagetable)();
	void (*yield)();

	void* (*kalloc)();
	void (*kfree)(void* pa);
	
	int (*copyin)(pagetable_t pagetable, char* dst, uint64 srcva, uint64 len);
	int (*copyout)(pagetable_t pagetable, uint64 dstva, char* src, uint64 len);
};

void set_pipe(struct pipe_context *pipe_context);

#endif // PIPE_H