#ifndef DISK_DEPENDENCY_H
#define DISK_DEPENDENCY_H

#include "defs.h"

extern int PID;

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();
extern void *memset(void *, int, uint);

#endif  //! DISK_DEPENDENCY_H
