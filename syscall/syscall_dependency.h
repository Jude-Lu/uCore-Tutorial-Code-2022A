#ifndef SYSCALL_DEPENDENCY_H
#define SYSCALL_DEPENDENCY_H

extern void printf(char*, ...);
extern int procid();
extern int threadid();
extern void dummy(int, ...);
extern void shutdown();

#endif  //! SYSCALL_DEPENDENCY_H
