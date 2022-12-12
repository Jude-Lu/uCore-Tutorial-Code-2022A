#ifndef EXTERN_H
#define EXTERN_H

extern char INIT_PROC[];
extern char e_text[];
extern char trampoline[];
extern char s_bss[];
extern char e_bss[];
extern char trampoline[], uservec[], kernelvec[], userret[];
extern char boot_stack_top[];

#endif // EXTERN_H
