#ifndef OS4_TRAP_H
#define OS4_TRAP_H

#include "defs.h"

void os4_set_usertrap();
void os4_set_kerneltrap();

struct trapframe* os4_get_trapframe();
uint64 os4_get_kernel_sp();

void os4_call_userret();
void os4_finish_usertrap(int cause);
void os4_error_in_trap(int status);

void os4_super_external_handler();

void trap_init();

#endif // OS4_TRAP_H