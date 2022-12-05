
#include "signal.h"

struct signal_context *sig_context;

void set_signal(struct signal_context *signal_context)
{
	sig_context = signal_context;
}

int sigaction(uint32 signum, uint64 va_act, uint64 va_oldact)
{
	if (signum < 1U || signum > 31U)
		return -1;
	if (signum == SIGKILL || signum == SIGSTOP || signum == SIGCONT) // 这三种信号必须由内核处理
		return -1;
	struct sigaction *action = (sig_context->get_curr_sig_block)()->sig_actions + signum;
	if (va_oldact && sig_context->copyout(sig_context->get_curr_pagetable(), va_oldact, (char*)action, sizeof(struct sigaction)) < 0)
		return -1;
	if (sig_context->copyin(sig_context->get_curr_pagetable(), (char*)action, va_act, sizeof(struct sigaction)) < 0)
		return -1;
	return 0;
}

int sigprocmask(uint32 mask)
{
	(sig_context->get_curr_sig_block)()->signal_mask = mask;
	return 0;
}

int sigreturn()
{
	struct signal_block *sig_block = (sig_context->get_curr_sig_block)();
	if (!sig_block->handling_sig)
		return -1;
	sig_block->signals -= (1U << sig_block->handling_sig);
	sig_block->handling_sig = 0;
	sig_context->customized_sigreturn();
	return 0;
}

int sigkill(int pid, uint32 signum)
{
	if (signum < 1U || signum > 31U)
		return -1;
	
	struct signal_block *sig_block = (sig_context->pid2sig_block)(pid);
	if (sig_block == NULL || sig_block->killed)
		return -1;
	
	// 如果该进程正在处理某个信号，那么屏蔽的信号就是它对应sigaction数据结构的sa_mask；反之就是进程signal_block数据结构的signal_mask
	uint32 mask;
	if (sig_block->handling_sig)
		mask = sig_block->sig_actions[sig_block->handling_sig].sa_mask;
	else
		mask = sig_block->signal_mask;
	
	if (mask & (1U << signum))
		return -1;
	else
	{
		if (signum == SIGKILL)
		{
			sig_block->killed = 1; // 目前的设计是，等到调度到对应进程的某个线程时杀死该进程，而不是立即处理
			sig_block->frozen = 0; // 认为这个进程一定没有被冻住，否则会调度不到该进程
		}
		else if (signum == SIGCONT)
			sig_block->frozen = 0;
		else if (signum == SIGSTOP)
			sig_block->frozen = 1;
		else
			sig_block->signals |= (1U << signum); // 在signal里面标记好需要处理signum这个信号，交由用户注册的signal handler处理
		return 0;
	}
}