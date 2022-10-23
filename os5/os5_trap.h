#ifndef OS5_TRAP_H
#define OS5_TRAP_H

#include "defs.h"

void os5_set_usertrap();
void os5_set_kerneltrap();

struct trapframe* os5_get_trapframe();
uint64 os5_get_kernel_sp();

void os5_call_userret();
void os5_finish_usertrap(int cause);
void os5_error_in_trap(int status);

void os5_super_external_handler();

void trap_init();

#endif // OS5_TRAP_H