#ifndef EXTERN_H
#define EXTERN_H

#include "defs.h"

static int app_num;
static uint64 *app_info_ptr;
extern char _app_num[], _app_names[], INIT_PROC[];
char names[MAX_APP_NUM][MAX_STR_LEN];
extern char e_text[];
extern char trampoline[];
extern char s_bss[];
extern char e_bss[];
extern char trampoline[], uservec[], kerneltrap[], userret[];
extern char boot_stack_top[];

#endif // EXTERN_H
