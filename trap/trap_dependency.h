#ifndef TRAP_DEPENDENCY_H
#define TRAP_DEPENDENCY_H

#include "defs.h"

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();
extern void set_timer(uint64);

#endif  //! TRAP_DEPENDENCY_H
