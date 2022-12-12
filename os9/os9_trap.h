#ifndef OS8_TRAP_H
#define OS8_TRAP_H

#include "modules.h"

void os9_set_usertrap();
void os9_set_kerneltrap();

struct trapframe* os9_get_trapframe();
uint64 os9_get_kernel_sp();

void os9_call_userret();
void os9_customized_usertrap(int cause);
void os9_error_in_trap(int status);

void os9_super_external_handler();

void trap_init();

#endif // OS8_TRAP_H