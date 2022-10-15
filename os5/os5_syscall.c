#include "os5_syscall.h"
#include "../utils/defs.h"
#include "../syscall/syscall_ids.h"
#include "loader.h"
#include "os5_trap.h"

uint64 os5_sys_write(int fd, uint64 va, uint64 len)
{
	debugf("sys_write fd = %d str = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_task();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
}

uint64 os5_sys_read(int fd, uint64 va, uint64 len)
{
	debugf("sys_read fd = %d str = %x, len = %d", fd, va, len);
	if (fd != STDIN)
		return -1;
	struct proc *p = curr_task();
	char str[MAX_STR_LEN];
	for (int i = 0; i < len; ++i) {
		int c = consgetc();
		str[i] = c;
	}
	copyout(p->pagetable, va, str, len);
	return len;
}

__attribute__((noreturn)) void os5_sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 os5_sys_sched_yield()
{
	yield();
	return 0;
}

uint64 os5_sys_gettimeofday(uint64 val, int _tz)
{
	struct proc *p = curr_task();
	uint64 cycle = get_cycle();
	TimeVal t;
	t.sec = cycle / CPU_FREQ;
	t.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, val, (char *)&t, sizeof(TimeVal));
	return 0;
}

uint64 os5_sys_getpid()
{
	return ((struct proc*)curr_task())->pid;
}

uint64 os5_sys_getppid()
{
	struct proc *p = curr_task();
	return p->parent == NULL ? IDLE_PID : p->parent->pid;
}

uint64 os5_sys_clone()
{
	debugf("fork!\n");
	return fork();
}

uint64 os5_sys_exec(uint64 va, uint64 uargv)
{
	struct proc *p = curr_task();
	char name[200];
	copyinstr(p->pagetable, name, va, 200);
	debugf("sys_exec %s\n", name);
	return exec(name);
}

uint64 os5_sys_wait(int pid, uint64 va)
{
	struct proc *p = curr_task();
	int *code = (int *)useraddr(p->pagetable, va);
	return wait(pid, code);
}

uint64 os5_sys_spawn(uint64 va)
{
	// TODO: your job is to complete the sys call
	return -1;
}

uint64 os5_sys_set_priority(long long prio){
	// TODO: your job is to complete the sys call
	return -1;
}

void syscall_init()
{
	static struct syscall_context os5_sys_context =
	{
		.sys_write = os5_sys_write,
		.sys_read = os5_sys_read,
		.sys_exit = os5_sys_exit,
		.sys_sched_yield = os5_sys_sched_yield,
		.sys_gettimeofday = os5_sys_gettimeofday,
		.sys_getpid = os5_sys_getpid,
		.sys_getppid = os5_sys_getppid,
		.sys_clone = os5_sys_clone,
		.sys_exec = os5_sys_exec,
		.sys_wait = os5_sys_wait,
		.sys_spawn = os5_sys_spawn,
		.sys_set_priority = os5_sys_set_priority
	};
	set_syscall(&os5_sys_context);
}
