#include "syscall.h"
#include "syscall_ids.h"
#include "../trap/trap.h"

struct syscall_context* sys_context;

/// 初始化sys_context
void set_syscall(struct syscall_context* syscall_context) {
	sys_context = syscall_context;
}

/// syscall函数从上下文中获取对应的syscall id，分别调用对应的syscall处理函数，而对应的syscall处理函数已经在sys_context中指定
void syscall(struct trapframe* trapframe) {
	int id = trapframe->a7, ret = 0;
	uint64 args[6] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
	switch (id) {
		// ch2
		case SYS_write:
			ret = sys_context->sys_write(args[0], args[1], args[2]);
			break;
		case SYS_exit:
			sys_context->sys_exit(args[0]);

		// ch3
		case SYS_sched_yield:
			ret = sys_context->sys_sched_yield();
			break;
		case SYS_gettimeofday:
			ret = sys_context->sys_gettimeofday(args[0], args[1]);
			break;

		// ch5
		case SYS_read:
			ret = sys_context->sys_read(args[0], args[1], args[2]);
			break;
		case SYS_getpid:
			ret = sys_context->sys_getpid();
			break;
		case SYS_getppid:
			ret = sys_context->sys_getppid();
			break;
		case SYS_clone:  // SYS_fork
			ret = sys_context->sys_clone();
			break;
		case SYS_execve:
			ret = sys_context->sys_exec(args[0], args[1]);
 			break;
		case SYS_wait4:
			ret = sys_context->sys_wait(args[0], args[1]);
			break;
		case SYS_spawn:
			ret = sys_context->sys_spawn(args[0]);
			break;
        case SYS_setpriority:
			ret = sys_context->sys_set_priority(args[0]);
			break;

		// ch6
		case SYS_openat:
			ret = sys_context->sys_openat(args[0], args[1], args[2]);
			break;
		case SYS_close:
			ret = sys_context->sys_close(args[0]);
			break;
		case SYS_fstat:
			ret = sys_context->sys_fstat(args[0], args[1]);
			break;
		case SYS_linkat:
			ret = sys_context->sys_linkat(args[0], args[1], args[2], args[3], args[4]);
			break;
		case SYS_unlinkat:
			ret = sys_context->sys_unlinkat(args[0], args[1], args[2]);
			break;

		// ch7
		case SYS_pipe2:
			ret = sys_context->sys_pipe(args[0]);
			break;

		// ch8
		case SYS_thread_create:
			ret = sys_context->sys_thread_create(args[0], args[1]);
			break;
		case SYS_gettid:
			ret = sys_context->sys_gettid();
			break;
		case SYS_waittid:
			ret = sys_context->sys_waittid(args[0]);
			break;
		case SYS_mutex_create:
			ret = sys_context->sys_mutex_create(args[0]);
			break;
		case SYS_mutex_lock:
			ret = sys_context->sys_mutex_lock(args[0]);
			break;
		case SYS_mutex_unlock:
			ret = sys_context->sys_mutex_unlock(args[0]);
			break;
		case SYS_semaphore_create:
			ret = sys_context->sys_semaphore_create(args[0]);
			break;
		case SYS_semaphore_up:
			ret = sys_context->sys_semaphore_up(args[0]);
			break;
		case SYS_semaphore_down:
			ret = sys_context->sys_semaphore_down(args[0]);
			break;
		case SYS_condvar_create:
			ret = sys_context->sys_condvar_create();
			break;
		case SYS_condvar_signal:
			ret = sys_context->sys_condvar_signal(args[0]);
			break;
		case SYS_condvar_wait:
			ret = sys_context->sys_condvar_wait(args[0], args[1]);
			break;
		case SYS_enable_deadlock_detect:
			ret = sys_context->sys_enable_deadlock_detect(args[0]);
			break;
		default:
			ret = -1;
			errorf("unknown syscall %d", id);
	}
	trapframe->a0 = ret;
}