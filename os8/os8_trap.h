#ifndef OS8_TRAP_H
#define OS8_TRAP_H

#include "modules.h"

void os8_set_usertrap();
void os8_set_kerneltrap();

struct trapframe* os8_get_trapframe();
uint64 os8_get_kernel_sp();

void os8_call_userret();
void os8_finish_usertrap(int cause);
void os8_error_in_trap(int status);

void os8_super_external_handler();

void trap_init();

#endif // OS8_TRAP_H