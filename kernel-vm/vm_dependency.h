#ifndef VM_DEPENDENCY_H
#define VM_DEPENDENCY_H

#include "defs.h"

extern char ekernel[];
extern char trampoline[];

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();
extern void *memmove(void *, const void *, uint);
extern void *memset(void *, int, uint);

#endif  //! VM_DEPENDENCY_H
