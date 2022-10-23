#ifndef OS7_TRAP_H
#define OS7_TRAP_H

#include "modules.h"

void os7_set_usertrap();
void os7_set_kerneltrap();

struct trapframe* os7_get_trapframe();
uint64 os7_get_kernel_sp();

void os7_call_userret();
void os7_finish_usertrap(int cause);
void os7_error_in_trap(int status);

void os7_super_external_handler();

void trap_init();

#endif // OS7_TRAP_H