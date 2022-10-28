#ifndef PROC_H
#define PROC_H

#include "modules.h"

#define NPROC (16)

// Per-process state
struct proc {
	enum procstate state; ///< Process state
	int pid; ///< Process ID
	uint64 ustack; ///< Virtual address of user stack
	uint64 kstack; ///< Virtual address of kernel stack
	struct trapframe *trapframe; ///< data page for trampoline.S
	struct context context; ///< swtch() here to run process
	/*
	* LAB1: you may need to add some new fields here
	*/
};

/*
* LAB1: you may need to define struct for TaskInfo here
*/

void exit(int);
void proc_init();
void scheduler() __attribute__((noreturn));
void sched();
void yield();
// swtch.S
void swtch(struct context *, struct context *);

#endif // PROC_H