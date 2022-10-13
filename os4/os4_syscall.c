#include "loader.h"
#include "proc.h"
#include "os4_trap.h"
#include "../utils/defs.h"

uint64 os4_sys_write(int fd, uint64 va, uint64 len)
{
	debugf("sys_write fd = %d va = %x, len = %d", fd, va, len);
	if (fd != STDOUT)
		return -1;
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	debugf("size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return size;
}

__attribute__((noreturn)) void os4_sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 os4_sys_sched_yield()
{
	yield();
	return 0;
}

uint64 os4_sys_gettimeofday(uint64 val, int _tz) // TODO: implement sys_gettimeofday in pagetable. (VA to PA)
{
	// YOUR CODE
	TimeVal* t = (TimeVal*)val;
	t->sec = 0;
	t->usec = 0;

	/* The code in `ch3` will leads to memory bugs*/

	// uint64 cycle = get_cycle();
	// t->sec = cycle / CPU_FREQ;
	// t->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	return 0;
}

// TODO: add support for mmap and munmap syscall.
// hint: read through docstrings in vm.c. Watching CH4 video may also help.
// Note the return value and PTE flags (especially U,X,W,R)
/*
* LAB1: you may need to define sys_task_info here
*/

void syscall_init()
{
    static struct syscall_context os4_sys_context =
    {
        .sys_write = os4_sys_write,
        .sys_exit = os4_sys_exit,
		.sys_sched_yield = os4_sys_sched_yield,
        .sys_gettimeofday = os4_sys_gettimeofday
	};
    set_syscall(&os4_sys_context);
}
