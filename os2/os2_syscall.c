#include "os2_syscall.h"
#include "loader.h"
#include "../utils/defs.h"

uint64 os2_sys_write(int fd, uint64 argument_str, uint64 len)
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

__attribute__((noreturn)) void os2_sys_exit(int code)
{
	debugf("sysexit(%d)", code);
	run_next_app();
	printf("ALL DONE\n");
	shutdown();
	__builtin_unreachable();
}

void syscall_init()
{
    static struct syscall_context os2_sys_context = 
	{
        .sys_write = os2_sys_write,
        .sys_exit = os2_sys_exit
	};
    set_syscall(&os2_sys_context);
}