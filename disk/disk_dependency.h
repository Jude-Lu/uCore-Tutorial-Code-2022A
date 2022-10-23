#ifndef DISK_DEPENDENCY_H
#define DISK_DEPENDENCY_H

#include "string.h"

extern int PID;

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();

#endif  //! DISK_DEPENDENCY_H
