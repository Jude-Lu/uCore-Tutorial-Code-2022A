#include "os2_trap.h"
#include "loader.h"

extern char trampoline[], uservec[], boot_stack_top[];
extern void *userret(uint64);
extern char trap_page[];

void os2_yield()
{
	// We do not support "yield" in ch2, so we do nothing here.
}

void os2_set_usertrap()
{
	// The function has been done in trap_init(), and we do not use os2_set_kerneltrap (see below),
	// so we do nothing here.
}

void os2_set_kerneltrap()
{
	// We do not encounter trap that occurs in S mode, so we do nothing here.
}

struct trapframe* os2_get_trapframe()
{
	return (struct trapframe*)trap_page;
}

uint64 os2_get_kernel_sp()
{
	return (uint64)boot_stack_top + PGSIZE;
}

void os2_call_userret()
{
	userret((uint64)os2_get_trapframe());
}

void os2_finish_user_trap(int cause)
{
	if (cause != UserEnvCall)
	{
		infof("switch to next app");
		run_next_app();
		printf("ALL DONE\n");
		shutdown();
	}
	else
		usertrapret();
}

void os2_error_in_trap(int status)
{
	// We are not able to do anything (e.g., kill the current process), just ignore the.
}

void os2_supervisorexternal_handler()
{
	// We do not encounter external interrupt in ch2, so we do nothing here.
}

void trap_init()
{
	static struct trap_handler_context os2_trap_context = 
	{
		.yield = os2_yield,
		
		.set_usertrap = os2_set_usertrap,
		.set_kerneltrap = os2_set_kerneltrap,

		.get_trapframe = os2_get_trapframe,
		.get_kernel_sp = os2_get_kernel_sp,
		
		.call_userret = os2_call_userret,
		.finish_usertrap = os2_finish_user_trap,
		.error_in_trap = os2_error_in_trap,

		.supervisorexternal_handler = os2_supervisorexternal_handler
	};
	set_trap(&os2_trap_context);

	// set up to take exceptions and traps while in the kernel.
	w_stvec((uint64)uservec & ~0x3);
}