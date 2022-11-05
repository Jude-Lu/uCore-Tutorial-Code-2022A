#ifndef OS2_TRAP_H
#define OS2_TRAP_H

#include "modules.h"

void os2_yield();

void os2_set_usertrap();
void os2_set_kerneltrap();

struct trapframe* os2_get_trapframe();
uint64 os2_get_kernel_sp();

void os2_call_userret();
void os2_customized_usertrap(int cause);
void os2_error_in_trap(int status);

void os2_super_external_handler();

void trap_init();

#endif // OS2_TRAP_H