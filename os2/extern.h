#ifndef EXTERN_H
#define EXTERN_H

#include "defs.h"

static int app_cur, app_num;
static uint64* app_info_ptr;
extern char s_bss[];
extern char e_bss[];
extern char _app_num[], userret[], boot_stack_top[], ekernel[];
extern char uservec[], userret[];
extern char trap_page[], boot_stack_top[];

#endif // EXTERN_H
