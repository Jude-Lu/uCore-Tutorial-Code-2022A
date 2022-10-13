#include "os3_trap.h"
#include "loader.h"
#include "proc.h"

extern char trampoline[], uservec[];
extern void *userret(uint64);

void os3_set_usertrap()
{
	w_stvec((uint64)uservec & ~0x3);
}

void os3_set_kerneltrap()
{
	w_stvec((uint64)kerneltrap & ~0x3);
}

struct trapframe* os3_get_trapframe()
{
	return curr_proc()->trapframe;
}

uint64 os3_get_kernel_sp()
{
	return curr_proc()->kstack + PGSIZE;
}

void os3_call_userret()
{
	userret((uint64)os3_get_trapframe());
}

void os3_finish_usertrap(int cause)
{
	usertrapret();
}

void os3_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os3_supervisorexternal_handler()
{
	// We do not encounter external interrupt in ch3, so we do nothing here.
}

void trap_init()
{
	static struct trap_handler_context os3_trap_context = 
	{
		.yield = yield,
		
		.set_usertrap = os3_set_usertrap,
		.set_kerneltrap = os3_set_kerneltrap,

		.get_trapframe = os3_get_trapframe,
		.get_kernel_sp = os3_get_kernel_sp,
		
		.call_userret = os3_call_userret,
		.finish_usertrap = os3_finish_usertrap,
		.error_in_trap = os3_error_in_trap,

		.supervisorexternal_handler = os3_supervisorexternal_handler
	};
	set_trap(&os3_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os3_set_kerneltrap();
}