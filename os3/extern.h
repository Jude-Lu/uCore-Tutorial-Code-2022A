#ifndef EXTERN_H
#define EXTERN_H

extern char s_bss[];
extern char e_bss[];
extern char _app_num[], ekernel[];
extern char uservec[], userret[], kerneltrap[];
extern char boot_stack_top[];

#endif // EXTERN_H
