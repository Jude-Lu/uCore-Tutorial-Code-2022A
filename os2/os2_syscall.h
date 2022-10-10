#ifndef OS2_SYSCALL_H
#define OS2_SYSCALL_H

#include "../utils/types.h"

uint64 os2_sys_write(int, uint64, uint64);
__attribute__((noreturn)) void os2_sys_exit(int);
void syscall_init();

#endif