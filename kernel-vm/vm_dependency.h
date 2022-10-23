#ifndef VM_DEPENDENCY_H
#define VM_DEPENDENCY_H

#include "string.h"

extern char ekernel[];
extern char trampoline[];

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();

#endif  //! VM_DEPENDENCY_H
