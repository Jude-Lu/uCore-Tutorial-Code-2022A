#include "loader.h"
#include "proc.h"
#include "os3_trap.h"
#include "os3_syscall.h"

void clean_bss()
{
	memset(s_bss, 0, e_bss - s_bss);
}

void main()
{
	clean_bss();
	proc_init();
	loader_init();
	trap_init();
	timer_init();
	syscall_init();
	run_all_app();
	infof("start scheduler!");
	scheduler();
}
