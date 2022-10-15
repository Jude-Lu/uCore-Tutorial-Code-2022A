#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "../utils/types.h"

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

enum threadstate { T_UNUSED, T_USED, T_SLEEPING, T_RUNNABLE, T_RUNNING, T_EXITED };

// Saved registers for kernel context switches.
struct context {
	uint64 ra;
	uint64 sp;

	// callee-saved
	uint64 s0;
	uint64 s1;
	uint64 s2;
	uint64 s3;
	uint64 s4;
	uint64 s5;
	uint64 s6;
	uint64 s7;
	uint64 s8;
	uint64 s9;
	uint64 s10;
	uint64 s11;
};

struct manager {
    void* (*create)();
    void (*remove)(void* p);
    void* (*get)(int id);
	int (*change)(void* p);
    void (*add)(void* p);
    void* (*fetch)(); 
};

void set_manager(struct manager*);
void* curr_task();
void set_curr(void*);
void* get_task(int);
int change_task(void*);
void* alloc_task();
void free_task(void*);
void add_task(void*);
void* fetch_task();

#endif // PROCESSOR_H
