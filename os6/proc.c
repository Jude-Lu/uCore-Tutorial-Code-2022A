#include "proc.h"
#include "loader.h"
#include "os6_trap.h"

struct proc pool[NPROC];
__attribute__((aligned(16))) char kstack[NPROC][PAGE_SIZE];
__attribute__((aligned(4096))) char trapframe[NPROC][TRAP_PAGE_SIZE];

struct proc idle;
struct queue task_queue;

int filepool_size = FILEPOOLSIZE;
struct file filepool[FILEPOOLSIZE];  // This is a system-level open file table that holds open files of all process.

int procid()
{
	return ((struct proc*)curr_task())->pid;
}

int threadid()
{
	return ((struct proc*)curr_task())->pid;
}

int cpuid()
{
	return 0;
}

int allocpid()
{
	static int PID = 1;
	return PID++;
}

// Free a process's page table, and free the
// physical memory it refers to.
void freepagetable(pagetable_t pagetable, uint64 max_page)
{
	uvmunmap(pagetable, TRAMPOLINE, 1, 0);
	uvmunmap(pagetable, TRAPFRAME, 1, 0);
	uvmfree(pagetable, max_page);
}

void* fetch()
{
	int index = pop_queue(&task_queue);
	if (index < 0) {
		debugf("No task to fetch\n");
		return NULL;
	}
	debugf("fetch task %d(pid=%d) from task queue\n", index,
	       pool[index].pid);
	return pool + index;
}

void add(void* proc)
{
	struct proc* p = (struct proc*)proc;
	p->state = RUNNABLE;
	push_queue(&task_queue, p - pool);
	debugf("add task %d(pid=%d) to task queue\n", p - pool, p->pid);
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
	// init proc
	p->pid = allocpid();
	p->state = USED;
	p->ustack = 0;
	p->max_page = 0;
	p->parent = NULL;
	p->exit_code = 0;
	p->pagetable = uvmcreate();
	if (mappages(p->pagetable, TRAPFRAME, PGSIZE, (uint64)p->trapframe, PTE_R | PTE_W) < 0) {
		panic("map trapframe fail");
	}
	memset(&p->context, 0, sizeof(p->context));
	memset((void *)p->kstack, 0, KSTACK_SIZE);
	memset((void *)p->trapframe, 0, TRAP_PAGE_SIZE);
	memset((void *)p->files, 0, sizeof(struct file *) * FD_BUFFER_SIZE);
	p->context.ra = (uint64)usertrapret;
	p->context.sp = p->kstack + KSTACK_SIZE;
	return p;
}

void remove(void* proc)
{
	struct proc* p = (struct proc*)proc;
	if (p->pagetable)
		freepagetable(p->pagetable, p->max_page);
	p->pagetable = 0;
	for (int i = 0; i > FD_BUFFER_SIZE; i++) {
		if (p->files[i] != NULL) {
			fileclose(p->files[i]);
		}
	}
	p->state = UNUSED;
}

int init_stdio(struct proc *p)
{
	for (int i = 0; i < 3; i++) {
		if (p->files[i] != NULL) {
			return -1;
		}
		p->files[i] = stdio_init(i);
	}
	return 0;
}

// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void scheduler()
{
	struct proc *p;
	for (;;) {
		p = fetch_task();
		if (p == NULL) {
			panic("all app are over!\n");
		}
		tracef("swtich to proc %d", p - pool);
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
void sched()
{
	struct proc *p = curr_task();
	if (p->state == RUNNING)
		panic("sched running");
	swtch(&p->context, &idle.context);
}

// Give up the CPU for one scheduling round.
void yield()
{
	add_task(curr_task());
	sched();
}

int fork()
{
	struct proc *np;
	struct proc *p = curr_task();
	int i;
	// Allocate process.
	if ((np = alloc_task()) == 0) {
		panic("allocproc\n");
	}
	// Copy user memory from parent to child.
	if (uvmcopy(p->pagetable, np->pagetable, p->max_page) < 0) {
		panic("uvmcopy\n");
	}
	np->max_page = p->max_page;
	// Copy file table to new proc
	for (i = 0; i < FD_BUFFER_SIZE; i++) {
		if (p->files[i] != NULL) {
			// TODO: f->type == STDIO ?
			p->files[i]->ref++;
			np->files[i] = p->files[i];
		}
	}
	// copy saved user registers.
	*(np->trapframe) = *(p->trapframe);
	// Cause fork to return 0 in the child.
	np->trapframe->a0 = 0;
	np->parent = p;
	add_task(np);
	return np->pid;
}

int push_argv(struct proc *p, char **argv)
{
	uint64 argc, ustack[MAX_ARG_NUM + 1];
	uint64 sp = p->ustack + USTACK_SIZE, spb = p->ustack;
	// Push argument strings, prepare rest of stack in ustack.
	for (argc = 0; argv[argc]; argc++) {
		if (argc >= MAX_ARG_NUM)
			panic("...");
		sp -= strlen(argv[argc]) + 1;
		sp -= sp % 16; // riscv sp must be 16-byte aligned
		if (sp < spb) {
			panic("...");
		}
		if (copyout(p->pagetable, sp, argv[argc],
			    strlen(argv[argc]) + 1) < 0) {
			panic("...");
		}
		ustack[argc] = sp;
	}
	ustack[argc] = 0;
	// push the array of argv[] pointers.
	sp -= (argc + 1) * sizeof(uint64);
	sp -= sp % 16;
	if (sp < spb) {
		panic("...");
	}
	if (copyout(p->pagetable, sp, (char *)ustack,
		    (argc + 1) * sizeof(uint64)) < 0) {
		panic("...");
	}
	p->trapframe->a1 = sp;
	p->trapframe->sp = sp;
	// clear files ?
	return argc; // this ends up in a0, the first argument to main(argc, argv)
}

int exec(char *path, char **argv)
{
	infof("exec : %s\n", path);
	struct inode *ip;
	struct proc *p = curr_task();
	if ((ip = namei(path)) == 0) {
		errorf("invalid file name %s\n", path);
		return -1;
	}
	uvmunmap(p->pagetable, 0, p->max_page, 1);
	bin_loader(ip, p);
	iput(ip);
	return push_argv(p, argv);
}

int wait(int pid, int *code)
{
	struct proc *np;
	int havekids;
	struct proc *p = curr_task();

	for (;;) {
		// Scan through table looking for exited children.
		havekids = 0;
		for (np = pool; np < &pool[NPROC]; np++) {
			if (np->state != UNUSED && np->parent == p &&
			    (pid <= 0 || np->pid == pid)) {
				havekids = 1;
				if (np->state == ZOMBIE) {
					// Found one.
					np->state = UNUSED;
					pid = np->pid;
					*code = np->exit_code;
					return pid;
				}
			}
		}
		if (!havekids) {
			return -1;
		}
		add_task(p);
		sched();
	}
}

// Exit the current process.
void exit(int code)
{
	struct proc *p = curr_task();
	p->exit_code = code;
	debugf("proc %d exit with %d", p->pid, code);
	free_task(p);
	if (p->parent != NULL) {
		// Parent should `wait`
		p->state = ZOMBIE;
	}
	// Set the `parent` of all children to NULL
	struct proc *np;
	for (np = pool; np < &pool[NPROC]; np++) {
		if (np->parent == p) {
			np->parent = NULL;
		}
	}
	sched();
}

int fdalloc(struct file *f)
{
	debugf("debugf f = %p, type = %d", f, f->type);
	struct proc *p = curr_task();
	for (int i = 0; i < FD_BUFFER_SIZE; ++i) {
		if (p->files[i] == NULL) {
			p->files[i] = f;
			debugf("debugf fd = %d, f = %p", i, p->files[i]);
			return i;
		}
	}
	return -1;
}

//Add a new system-level table entry for the open file table
struct file *filealloc()
{
	for (int i = 0; i < filepool_size; ++i) {
		if (filepool[i].ref == 0) {
			filepool[i].ref = 1;
			return &filepool[i];
		}
	}
	return 0;
}

void* undefined_pipeopen()
{
	errorf("In ch6, we have not implemented pipe yet!");
	exit(-1);
	return 0;
}

void undefined_pipeclose(void *_pi, int writable)
{
	errorf("In ch6, we have not implemented pipe yet!");
	exit(-1);
}

pagetable_t get_curr_pagetable()
{
	return ((struct proc*)curr_task()) -> pagetable;
}

// initialize the proc table at boot time.
void proc_init()
{
	struct proc *p;
	for (p = pool; p < &pool[NPROC]; p++) {
		p->state = UNUSED;
		p->kstack = (uint64)kstack[p - pool];
		p->trapframe = (struct trapframe *)trapframe[p - pool];
	}
	idle.kstack = (uint64)boot_stack_top;
	idle.pid = IDLE_PID;
	set_curr(&idle);
	init_queue(&task_queue, QUEUE_SIZE, process_queue_data);
	static struct manager os6_manager =
	{
		.create = create,
		.remove = remove,
		.add = add,
		.fetch = fetch
	};
	set_manager(&os6_manager);

	static struct virtio_context os6_virtio = 
	{
		.yield = yield
	};
	set_virtio(&os6_virtio);

	static struct FSManager os6_fs_manager = 
	{
		.fdalloc = fdalloc,
		.filealloc = filealloc,
		.get_curr_pagetable = get_curr_pagetable,

		.either_copyout = either_copyout,
		.either_copyin = either_copyin,

		.pipeopen = undefined_pipeopen,
		.pipeclose = undefined_pipeclose,

		.bread = bread,
		.brelse = brelse,
		.bwrite = bwrite,
		.bpin = bpin,
		.bunpin = bunpin,
		.buf_data = buf_data
	};
	set_file(&os6_fs_manager);
}
