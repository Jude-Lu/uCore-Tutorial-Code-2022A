#include "os3_trap.h"
#include "loader.h"
#include "proc.h"

extern char uservec[], userret[], kerneltrap[];

int os3_cpuid()
{
	return 0;
}

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
	return ((struct proc*)curr_task())->trapframe;
}

uint64 os3_get_trapframe_va()
{
	// Actually, os3 does not have the concept of virtual address.
	return (uint64)os3_get_trapframe();
}

pagetable_t os3_get_satp()
{
	// Actually, os3 does not have pagetable.
	return 0;
}

uint64 os3_get_kernel_sp()
{
	return ((struct proc*)curr_task())->kstack + PGSIZE;
}

uint64 os3_get_userret()
{
	return (uint64)userret;
}

void os3_finish_usertrap(int cause)
{
	usertrapret();
}

void os3_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os3_super_external_handler()
{
	// We do not encounter external interrupt in ch3, so we do nothing here.
}

void trap_init()
{
	static struct trap_handler_context os3_trap_context = 
	{
		.yield = yield,

		.cpuid = os3_cpuid,
		
		.set_usertrap = os3_set_usertrap,
		.set_kerneltrap = os3_set_kerneltrap,

		.get_trapframe = os3_get_trapframe,
		.get_trapframe_va = os3_get_trapframe_va,
		.get_satp = os3_get_satp,
		.get_kernel_sp = os3_get_kernel_sp,
		
		.get_userret = os3_get_userret,
		.finish_usertrap = os3_finish_usertrap,
		.error_in_trap = os3_error_in_trap,

		.syscall = syscall,

		.super_external_handler = os3_super_external_handler
	};
	set_trap(&os3_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os3_set_kerneltrap();
}