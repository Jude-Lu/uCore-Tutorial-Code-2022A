#include "os7_trap.h"
#include "loader.h"
#include "proc.h"
#include "../trap/plic.h"
#include "../disk/virtio.h"

extern char trampoline[], uservec[];
extern char userret[], kernelvec[];

void os7_set_usertrap()
{
	w_stvec(((uint64)TRAMPOLINE + (uservec - trampoline)) & ~0x3);
}

void os7_set_kerneltrap()
{
	w_stvec((uint64)kernelvec & ~0x3);
}

struct trapframe* os7_get_trapframe()
{
	return ((struct proc*)curr_task())->trapframe;
}

uint64 os7_get_kernel_sp()
{
	return ((struct proc*)curr_task())->kstack + PGSIZE;
}

void os7_call_userret()
{
	struct trapframe *trapframe = ((struct proc*)curr_task())->trapframe;
	uint64 satp = MAKE_SATP(((struct proc*)curr_task())->pagetable);
	uint64 fn = TRAMPOLINE + (userret - trampoline);
	tracef("return to user @ %p", trapframe->epc);
	((void (*)(uint64, uint64))fn)(TRAPFRAME, satp);
}

void os7_finish_usertrap(int cause)
{
	usertrapret();
}

void os7_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os7_super_external_handler(int cpuid)
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
	static struct trap_handler_context os7_trap_context = 
	{
		.yield = yield,

		.cpuid = cpuid,
		
		.set_usertrap = os7_set_usertrap,
		.set_kerneltrap = os7_set_kerneltrap,

		.get_trapframe = os7_get_trapframe,
		.get_kernel_sp = os7_get_kernel_sp,
		
		.call_userret = os7_call_userret,
		.finish_usertrap = os7_finish_usertrap,
		.error_in_trap = os7_error_in_trap,

		.super_external_handler = os7_super_external_handler
	};
	set_trap(&os7_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os7_set_kerneltrap();
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}