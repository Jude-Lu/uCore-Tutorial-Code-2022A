#include "os3_syscall.h"
#include "os3_trap.h"
#include "loader.h"
#include "proc.h"

uint64 os3_sys_write(int fd, uint64 argument_str, uint64 len)
{
	char *str = (char*)argument_str;
	debugf("sys_write fd = %d str = %x, len = %d", fd, str, len);
	if (fd != STDOUT)
		return -1;
	for (int i = 0; i < len; ++i) {
		console_putchar(str[i]);
	}
	return len;
}

__attribute__((noreturn)) void os3_sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 os3_sys_sched_yield()
{
	yield();
	return 0;
}

uint64 os3_sys_gettimeofday(uint64 val, int _tz)
{
	TimeVal* t = (TimeVal*)val;
	uint64 cycle = get_cycle();
	t->sec = cycle / CPU_FREQ;
	t->usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	return 0;
}

/*
* LAB1: you may need to define sys_task_info here
*/

void syscall_init()
{
	static struct syscall_context os3_sys_context =
	{
		.sys_write = os3_sys_write,
		.sys_exit = os3_sys_exit,
		.sys_sched_yield = os3_sys_sched_yield,
		.sys_gettimeofday = os3_sys_gettimeofday
	};
	set_syscall(&os3_sys_context);
}
