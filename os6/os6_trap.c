#include "os6_trap.h"
#include "loader.h"
#include "proc.h"
#include "plic.h"
#include "virtio.h"

extern char trampoline[], uservec[];
extern char userret[], kernelvec[];

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
	return curr_proc()->trapframe;
}

uint64 os6_get_kernel_sp()
{
	return curr_proc()->kstack + PGSIZE;
}

void os6_call_userret()
{
	struct trapframe *trapframe = curr_proc()->trapframe;
	uint64 satp = MAKE_SATP(curr_proc()->pagetable);
	uint64 fn = TRAMPOLINE + (userret - trampoline);
	tracef("return to user @ %p", trapframe->epc);
	((void (*)(uint64, uint64))fn)(TRAPFRAME, satp);
}

void os6_finish_usertrap(int cause)
{
	usertrapret();
}

void os6_error_in_trap(int status)
{
	exit(status); // Kill the process.
}

void os6_super_external_handler()
{
	int irq = plic_claim();
	if (irq == UART0_IRQ) {
		// do nothing
	} else if (irq == VIRTIO0_IRQ) {
		virtio_disk_intr();
	} else if (irq) {
		infof("unexpected interrupt irq=%d\n", irq);
	}
	if (irq)
		plic_complete(irq);
}

void trap_init()
{
	static struct trap_handler_context os6_trap_context = 
	{
		.yield = yield,
		
		.set_usertrap = os6_set_usertrap,
		.set_kerneltrap = os6_set_kerneltrap,

		.get_trapframe = os6_get_trapframe,
		.get_kernel_sp = os6_get_kernel_sp,
		
		.call_userret = os6_call_userret,
		.finish_usertrap = os6_finish_usertrap,
		.error_in_trap = os6_error_in_trap,

		.super_external_handler = os6_super_external_handler
	};
	set_trap(&os6_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os6_set_kerneltrap();
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}