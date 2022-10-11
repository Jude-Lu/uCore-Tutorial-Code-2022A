#include "../utils/console.h"
#include "../utils/defs.h"
#include "loader.h"
#include "os2_trap.h"
#include "os2_syscall.h"

int procid()
{
	return 0;
}

int threadid()
{
	return 0;
}

void clean_bss()
{
	extern char s_bss[];
	extern char e_bss[];
	memset(s_bss, 0, e_bss - s_bss);
}

void main()
{
	clean_bss();
	printf("hello wrold!\n");
	trap_init();
	syscall_init();
    loader_init();
    run_next_app();
}
