#ifndef FS_DEPENDENCY_H
#define FS_DEPENDENCY_H

#include "defs.h"

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();
extern void *memmove(void *, const void *, uint);
extern void *memset(void *, int, uint);
extern int strncmp(const char *, const char *, uint);
extern char *strncpy(char *, const char *, int);

#endif  //! FS_DEPENDENCY_H
