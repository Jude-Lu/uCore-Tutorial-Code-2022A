#include "proc.h"
#include "loader.h"
#include "os3_trap.h"

struct proc pool[NPROC];
char kstack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char ustack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char trapframe[NPROC][PAGE_SIZE];

struct proc idle;

int procid()
{
	return ((struct proc*)curr_task())->pid;
}

int threadid()
{
	return ((struct proc*)curr_task())->pid;
}

int allocpid()
{
	static int PID = 1;
	return PID++;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel.
// If there are no free procs, or a memory allocation fails, return 0.
void* create() 
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		if (p->state == UNUSED) {
			goto found;
		}
	}
	return 0;

found:
	p->pid = allocpid();
	p->state = USED;
	memset(&p->context, 0, sizeof(p->context));
	memset(p->trapframe, 0, PAGE_SIZE);
	memset((void *)p->kstack, 0, PAGE_SIZE);
	p->context.ra = (uint64)usertrapret;
	p->context.sp = p->kstack + PAGE_SIZE;
	return p;
}

void remove(void* p)
{
	((struct proc*)p)->state = UNUSED;
	finished();
}

void add(void* p)
{
	((struct proc*)p)->state = RUNNABLE;
}

void* fetch()
{
	for (struct proc* p = pool; p < &pool[NPROC]; p++) {
		if (p->state == RUNNABLE) {
			/*
			* LAB1: you may need to init proc start time here
			*/
            return p;
        }
    }
    return 0;
}

// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler(void)
{
	for (;;) {
		struct proc *p = fetch_task();
		p->state = RUNNING;
		set_curr(p);
		swtch(&idle.context, &p->context);
	}
}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void sched(void)
{
	struct proc *p = curr_task();
	if (p->state == RUNNING)
		panic("sched running");
	swtch(&p->context, &idle.context);
}

// Give up the CPU for one scheduling round.
void yield(void)
{
    add_task(curr_task());
    sched();
}

// Exit the current process.
void exit(int code)
{
	struct proc *p = (struct proc*)curr_task();
	infof("proc %d exit with %d", p->pid, code);
	free_task(p);
	sched();
}

// initialize the proc table at boot time.
void proc_init(void)
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		p->state = UNUSED;
		p->kstack = (uint64)kstack[p - pool];
		p->ustack = (uint64)ustack[p - pool];
		p->trapframe = (struct trapframe *)trapframe[p - pool];
		/*
		* LAB1: you may need to initialize your new fields of proc here
		*/
	}
	idle.kstack = (uint64)boot_stack_top;
	idle.pid = 0;
	set_curr(&idle);
	static struct manager os3_manager =
	{
		.create = create,
		.remove = remove,
		.add = add,
		.fetch = fetch
	};
	set_manager(&os3_manager);
}
