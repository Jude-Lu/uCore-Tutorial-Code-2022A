#ifndef EXTERN_H
#define EXTERN_H

extern char _app_num[], _app_names[], INIT_PROC[];
extern char e_text[];
extern char trampoline[];
extern char s_bss[];
extern char e_bss[];
extern char trampoline[], uservec[], kerneltrap[], userret[];
extern char boot_stack_top[];

#endif // EXTERN_H
