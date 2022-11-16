#include "os9_trap.h"
#include "loader.h"
#include "proc.h"

void os9_set_usertrap()
{
	w_stvec(((uint64)TRAMPOLINE + (uservec - trampoline)) & ~0x3);
}

void os9_set_kerneltrap()
{
	w_stvec((uint64)kernelvec & ~0x3);
}

struct trapframe* os9_get_trapframe()
{
	return ((struct thread*)curr_task())->trapframe;
}

uint64 os9_get_trapframe_va()
{
	return get_thread_trapframe_va(((struct thread*)curr_task())->tid);
}

pagetable_t os9_get_satp()
{
	return (pagetable_t)MAKE_SATP(((struct thread*)curr_task())->process->pagetable);
}

uint64 os9_get_kernel_sp()
{
	return ((struct thread*)curr_task())->kstack + KSTACK_SIZE;
}

uint64 os9_get_userret()
{
	return TRAMPOLINE + (userret - trampoline);
}

void os9_customized_usertrap(int cause)
{
	struct signal_block *sig_block = &curr_proc()->sig_block;
	if (sig_block->killed)
	{
		exit_proc(137);
		__builtin_unreachable();
	}
	if (sig_block->frozen)
		yield();
	if (!sig_block->handling_sig)
		for (uint32 signum = 1U;signum <= 31U;++signum)
			if (sig_block->signals & (1U << signum))
			{
				struct trapframe* trapframe = ((struct thread*)curr_task())->trapframe;
				curr_proc()->sig_trapframe = *trapframe;
				sig_block->handling_sig = signum;
				trapframe->epc = (uint64)sig_block->sig_actions[signum].handler;
				break;
			}
	usertrapret();
}

void os9_error_in_trap(int status)
{
	exit(status); // Kill the thread.
}

void os9_super_external_handler(int cpuid)
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
	static struct trap_handler_context os9_trap_context = 
	{
		.yield = yield,

		.cpuid = cpuid,
		
		.set_usertrap = os9_set_usertrap,
		.set_kerneltrap = os9_set_kerneltrap,

		.get_trapframe = os9_get_trapframe,
		.get_trapframe_va = os9_get_trapframe_va,
		.get_satp = os9_get_satp,
		.get_kernel_sp = os9_get_kernel_sp,
		
		.get_userret = os9_get_userret,
		.customized_usertrap = os9_customized_usertrap,
		.error_in_trap = os9_error_in_trap,

		.syscall = syscall,

		.super_external_handler = os9_super_external_handler
	};
	set_trap(&os9_trap_context);

	// set up to take exceptions and traps while in the kernel.
	os9_set_kerneltrap();
	w_sie(r_sie() | SIE_SEIE | SIE_STIE | SIE_SSIE);
}