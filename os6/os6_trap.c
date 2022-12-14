#include "os6_trap.h"
#include "loader.h"
#include "proc.h"

void os6_set_usertrap()
{
	w_stvec(((uint64)TRAMPOLINE + (uservec - trampoline)) & ~0x3);
}

void os6_set_kerneltrap()
{
	w_stvec((uint64)kernelvec & ~0x3);
}

struct trapframe* os6_get_trapframe()
{
	return ((struct proc*)curr_task())->trapframe;
}

uint64 os6_get_trapframe_va()
{
	return TRAPFRAME;
}

pagetable_t os6_get_satp()
{
	return (pagetable_t)MAKE_SATP(((struct proc*)curr_task())->pagetable);
}

uint64 os6_get_kernel_sp()
{
	return ((struct proc*)curr_task())->kstack + PGSIZE;
}

uint64 os6_get_userret()
{
	return TRAMPOLINE + (userret - trampoline);
}

void os6_customized_usertrap(int cause)
{
	usertrapret();
}

void os6_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os6_super_external_handler(int cpuid)
{
	int irq = plic_claim(cpuid);
	if (irq == UART0_IRQ) {
		// do nothing
	} else if (irq == VIRTIO0_IRQ) {
		virtio_disk_intr();
	} else if (irq) {
		infof("unexpected interrupt irq=%d\n", irq);
	}
	if (irq)
		plic_complete(cpuid, irq);
}

void trap_init()
{
	static struct trap_handler_context os6_trap_context = 
	{
		.yield = yield,

		.cpuid = cpuid,
		
		.set_usertrap = os6_set_usertrap,
		.set_kerneltrap = os6_set_kerneltrap,

		.get_trapframe = os6_get_trapframe,
		.get_trapframe_va = os6_get_trapframe_va,
		.get_satp = os6_get_satp,
		.get_kernel_sp = os6_get_kernel_sp,
		
		.get_userret = os6_get_userret,
		.customized_usertrap = os6_customized_usertrap,
		.error_in_trap = os6_error_in_trap,

		.syscall = syscall,

		.super_external_handler = os6_super_external_handler
	};
	set_trap(&os6_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os6_set_kerneltrap();
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}