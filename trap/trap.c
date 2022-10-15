#include "../utils/defs.h"

struct trap_handler_context *trap_context;

void set_trap(struct trap_handler_context *trap_handler_context) {
	trap_context = trap_handler_context;
}

extern char uservec[], kernelvec[];

void unknown_trap()
{
	errorf("unknown trap: %p, stval = %p", r_scause(), r_stval());
	(trap_context->error_in_trap)(-1);
}

void devintr(uint64 cause)
{
	switch (cause) {
	case SupervisorTimer:
		set_next_timer();
		// if form user, allow yield
		if ((r_sstatus() & SSTATUS_SPP) == 0) {
			(trap_context->yield)();
		}
		break;
	case SupervisorExternal:
		(trap_context->super_external_handler)((trap_context->cpuid)());
		break;
	default:
		unknown_trap();
		break;
	}
}

void kerneltrap()
{
	uint64 sepc = r_sepc();
	uint64 sstatus = r_sstatus();
	uint64 scause = r_scause();

	debugf("kernel trap: epc = %p, cause = %d", sepc, scause);

	if ((sstatus & SSTATUS_SPP) == 0)
		panic("kerneltrap: not from supervisor mode");

	if (scause & (1ULL << 63)) {
		devintr(scause & 0xff);
	} else {
		errorf("invalid trap from kernel: %p, stval = %p sepc = %p\n",
		       scause, r_stval(), sepc);
		(trap_context->error_in_trap)(-1);
	}
	// the yield() may have caused some traps to occur,
	// so restore trap registers for use by kernelvec.S's sepc instruction.
	w_sepc(sepc);
	w_sstatus(sstatus);
}

void usertrap()
{
	(trap_context->set_kerneltrap)();
	struct trapframe *trapframe = (trap_context->get_trapframe)();
	tracef("trap from user epc = %p", trapframe->epc);
	if ((r_sstatus() & SSTATUS_SPP) != 0)
		panic("usertrap: not from user mode");

	uint64 cause = r_scause();
	if (cause & (1ULL << 63)) {
		devintr(cause & 0xff);
	} else {
		switch (cause) {
		case UserEnvCall:
			trapframe->epc += 4;
			syscall(trapframe);
			break;
		case StoreMisaligned:
		case StorePageFault:
		case InstructionMisaligned:
		case InstructionPageFault:
		case LoadMisaligned:
		case LoadPageFault:
			errorf("%d in application, bad addr = %p, bad instruction = %p, "
			       "core dumped.",
			       cause, r_stval(), trapframe->epc);
			(trap_context->error_in_trap)(-2);
			break;
		case IllegalInstruction:
			errorf("IllegalInstruction in application, core dumped.");
			(trap_context->error_in_trap)(-3);
			break;
		default:
			unknown_trap();
			break;
		}
	}
	(trap_context->finish_usertrap)(cause);
}

//
// return to user space
//
void usertrapret()
{
	(trap_context->set_usertrap)();
	struct trapframe *trapframe = (trap_context->get_trapframe)();
	trapframe->kernel_satp = r_satp(); // kernel page table
	trapframe->kernel_sp = (trap_context->get_kernel_sp)(); // process's kernel stack
	trapframe->kernel_trap = (uint64)usertrap;
	trapframe->kernel_hartid = r_tp(); // unuesd

	w_sepc(trapframe->epc);
	// set up the registers that trampoline.S's sret will use
	// to get to user space.

	// set S Previous Privilege mode to User.
	uint64 x = r_sstatus();
	x &= ~SSTATUS_SPP; // clear SPP to 0 for user mode
	x |= SSTATUS_SPIE; // enable interrupts in user mode
	w_sstatus(x);

	// Do some preprocessing, and then call userret in trampoline.S
	(trap_context->call_userret)();
}