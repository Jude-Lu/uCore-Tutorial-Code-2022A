#ifndef OS2_TRAP_H
#define OS2_TRAP_H

#include "../utils/defs.h"

void os2_yield();

void os2_set_usertrap();
void os2_set_kerneltrap();

struct trapframe* os2_get_trapframe();
uint64 os2_get_kernel_sp();

void os2_call_userret();
void os2_finish_user_trap(int cause);
void os2_error_in_trap(int status);

void os2_supervisorexternal_handler();

void trap_init();

#endif // OS2_TRAP_H