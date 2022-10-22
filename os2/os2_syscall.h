#ifndef OS2_SYSCALL_H
#define OS2_SYSCALL_H

#include "modules.h"

enum {
	STDIN = 0,
	STDOUT = 1,
	STDERR = 2,
};

void syscall_init();

#endif // OS2_SYSCALL_H