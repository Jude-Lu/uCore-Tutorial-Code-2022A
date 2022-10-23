#ifndef OS3_TRAP_H
#define OS3_TRAP_H

#include "modules.h"

void os3_set_usertrap();
void os3_set_kerneltrap();

struct trapframe* os3_get_trapframe();
uint64 os3_get_kernel_sp();

void os3_call_userret();
void os3_finish_usertrap(int cause);
void os3_error_in_trap(int status);

void os3_super_external_handler();

void trap_init();

#endif // OS3_TRAP_H