#ifndef OS2_TRAP_H
#define OS2_TRAP_H

#include "../utils/defs.h"
#include "os2_syscall.h"

void trap_init();
void usertrapret(struct trapframe *trapframe, uint64 kstack);

#endif // OS2_TRAP_H