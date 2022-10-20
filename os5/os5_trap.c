#include "os5_trap.h"
#include "loader.h"
#include "proc.h"

extern char trampoline[], uservec[], kerneltrap[];
extern char userret[];

int os5_cpuid()
{
	return 0;
}

void os5_set_usertrap()
{
	w_stvec(((uint64)TRAMPOLINE + (uservec - trampoline)) & ~0x3);
}

void os5_set_kerneltrap()
{
	w_stvec((uint64)kerneltrap & ~0x3);
}

struct trapframe* os5_get_trapframe()
{
	return ((struct proc*)curr_task())->trapframe;
}

uint64 os5_get_kernel_sp()
{
	return ((struct proc*)curr_task())->kstack + PGSIZE;
}

void os5_call_userret()
{
	struct trapframe *trapframe = ((struct proc*)curr_task())->trapframe;
	uint64 satp = MAKE_SATP(((struct proc*)curr_task())->pagetable);
	uint64 fn = TRAMPOLINE + (userret - trampoline);
	tracef("return to user @ %p", trapframe->epc);
	((void (*)(uint64, uint64))fn)(TRAPFRAME, satp);
}

void os5_finish_usertrap(int cause)
{
	usertrapret();
}

void os5_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os5_super_external_handler()
{
	// We do not encounter external interrupt in ch3, so we do nothing here.
}

void trap_init()
{
	static struct trap_handler_context os5_trap_context = 
	{
		.yield = yield,

		.cpuid = os5_cpuid,
		
		.set_usertrap = os5_set_usertrap,
		.set_kerneltrap = os5_set_kerneltrap,

		.get_trapframe = os5_get_trapframe,
		.get_kernel_sp = os5_get_kernel_sp,
		
		.call_userret = os5_call_userret,
		.finish_usertrap = os5_finish_usertrap,
		.error_in_trap = os5_error_in_trap,

		.syscall = syscall,

		.super_external_handler = os5_super_external_handler
	};
	set_trap(&os5_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os5_set_kerneltrap();
}