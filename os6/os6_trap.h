#ifndef OS6_TRAP_H
#define OS6_TRAP_H

#include "modules.h"

void os6_set_usertrap();
void os6_set_kerneltrap();

struct trapframe* os6_get_trapframe();
uint64 os6_get_kernel_sp();

void os6_call_userret();
void os6_finish_usertrap(int cause);
void os6_error_in_trap(int status);

void os6_super_external_handler();

void trap_init();

#endif // OS6_TRAP_H