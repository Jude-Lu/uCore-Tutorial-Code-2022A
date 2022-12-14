#include "os2_trap.h"
#include "loader.h"

void os2_yield()
{
	// We do not support "yield" in ch2, so we do nothing here.
}

int os2_cpuid()
{
	return 0;
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

uint64 os2_get_trapframe_va()
{
	// Actually, os2 does not have the concept of virtual address.
	return (uint64)trap_page;
}

pagetable_t os2_get_satp()
{
	// Actually, os2 does not have pagetable.
	return 0;
}

uint64 os2_get_kernel_sp()
{
	return (uint64)boot_stack_top + PGSIZE;
}

uint64 os2_get_userret()
{
	return (uint64)userret;
}

void os2_customized_usertrap(int cause)
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

void os2_super_external_handler()
{
	// We do not encounter external interrupt in ch2, so we do nothing here.
}

void trap_init()
{
	static struct trap_handler_context os2_trap_context = 
	{
		.yield = os2_yield,

		.cpuid = os2_cpuid,
		
		.set_usertrap = os2_set_usertrap,
		.set_kerneltrap = os2_set_kerneltrap,

		.get_trapframe = os2_get_trapframe,
		.get_trapframe_va = os2_get_trapframe_va,
		.get_satp = os2_get_satp,
		.get_kernel_sp = os2_get_kernel_sp,
		
		.get_userret = os2_get_userret,
		.customized_usertrap = os2_customized_usertrap,
		.error_in_trap = os2_error_in_trap,

		.syscall = syscall,

		.super_external_handler = os2_super_external_handler
	};
	set_trap(&os2_trap_context);

	// set up to take exceptions and traps while in the kernel.
	w_stvec((uint64)uservec & ~0x3);
}