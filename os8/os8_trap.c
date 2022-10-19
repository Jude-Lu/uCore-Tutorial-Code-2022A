#include "os8_trap.h"
#include "loader.h"
#include "proc.h"
#include "../trap/plic.h"
#include "../easy-fs/virtio.h"

extern char trampoline[], uservec[];
extern char userret[], kernelvec[];

void os8_set_usertrap()
{
	w_stvec(((uint64)TRAMPOLINE + (uservec - trampoline)) & ~0x3);
}

void os8_set_kerneltrap()
{
	w_stvec((uint64)kernelvec & ~0x3);
}

struct trapframe* os8_get_trapframe()
{
	return ((struct thread*)curr_task())->trapframe;
}

uint64 os8_get_kernel_sp()
{
	return ((struct thread*)curr_task())->kstack + KSTACK_SIZE;
}

void os8_call_userret()
{
	struct trapframe *trapframe = ((struct thread*)curr_task())->trapframe;
	uint64 satp = MAKE_SATP(((struct thread*)curr_task())->process->pagetable);
	uint64 fn = TRAMPOLINE + (userret - trampoline);
	uint64 trapframe_va = get_thread_trapframe_va(((struct thread*)curr_task())->tid);
	debugf("return to user @ %p, sp @ %p", trapframe->epc, trapframe->sp);
	((void (*)(uint64, uint64))fn)(trapframe_va, satp);
}

void os8_finish_usertrap(int cause)
{
	usertrapret();
}

void os8_error_in_trap(int status)
{
	exit(status); // Kill the thread.
}

void os8_super_external_handler(int cpuid)
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
	static struct trap_handler_context os8_trap_context = 
	{
		.yield = yield,

		.cpuid = cpuid,
		
		.set_usertrap = os8_set_usertrap,
		.set_kerneltrap = os8_set_kerneltrap,

		.get_trapframe = os8_get_trapframe,
		.get_kernel_sp = os8_get_kernel_sp,
		
		.call_userret = os8_call_userret,
		.finish_usertrap = os8_finish_usertrap,
		.error_in_trap = os8_error_in_trap,

		.super_external_handler = os8_super_external_handler
	};
	set_trap(&os8_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os8_set_kerneltrap();
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}