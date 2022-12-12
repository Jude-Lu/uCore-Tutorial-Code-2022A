#ifndef SYSCALL_H
#define SYSCALL_H

/**
 * @file syscall.h
 * @brief syscall模块主要负责系统调用的处理，根据传入的syscall id调用对应的syscall函数
*/

#include "defs.h"

/// syscall_context定义了各个syscall的函数指针，方便兼容不同章节syscall的差异，各个章节可以根据本章节的内容和难度实现本章节的syscall
struct syscall_context {
	// ch2 syscall
	uint64 (*sys_write)(int fd, uint64 va, uint64 len);
	void (*sys_exit)(int code);

	// ch3 新增syscall
	uint64 (*sys_sched_yield)();
	uint64 (*sys_gettimeofday)(uint64 val, int _tz);

	// ch5 新增syscall
	uint64 (*sys_read)(int fd, uint64 va, uint64 len);
	uint64 (*sys_getpid)();
	uint64 (*sys_getppid)();
	uint64 (*sys_clone)();
	uint64 (*sys_exec)(uint64 path, uint64 uargv);
	uint64 (*sys_wait)(int pid, uint64 va);
	/// 该syscall需学生在实验中实现
	uint64 (*sys_spawn)(uint64 va);
	/// 该syscall需学生在实验中实现
	uint64 (*sys_set_priority)(long long prio);

	// ch6 新增syscall
	uint64 (*sys_openat)(uint64 va, uint64 omode, uint64 _flags);
	uint64 (*sys_close)(int fd);
	/// 该syscall需学生在实验中实现
	int (*sys_fstat)(int fd,uint64 stat);
	/// 该syscall需学生在实验中实现
	int (*sys_linkat)(int olddirfd, uint64 oldpath, int newdirfd, uint64 newpath, uint64 flags);
	/// 该syscall需学生在实验中实现
	int (*sys_unlinkat)(int dirfd, uint64 name, uint64 flags);

	// ch7 新增syscall
	uint64 (*sys_pipe)(uint64 fdarray);

	// ch8 新增syscall
	int (*sys_thread_create)(uint64 entry, uint64 arg);
	int (*sys_gettid)();
	int (*sys_waittid)(int tid);
	int (*sys_mutex_create)(int blocking);
	int (*sys_mutex_lock)(int mutex_id);
	int (*sys_mutex_unlock)(int mutex_id);
	int (*sys_semaphore_create)(int res_count);
	int (*sys_semaphore_up)(int semaphore_id);
	int (*sys_semaphore_down)(int semaphore_id);
	int (*sys_condvar_create)();
	int (*sys_condvar_signal)(int cond_id);
	int (*sys_condvar_wait)(int cond_id, int mutex_id);
	/// 该syscall需学生在实验中实现
	int (*sys_enable_deadlock_detect)(int is_enable);

	// ch9 新增syscall
	int (*sys_sigaction)(uint32 signum, uint64 va_act, uint64 va_oldact);
	int (*sys_sigprocmask)(uint32 mask);
	int (*sys_sigkill)(int pid, uint32 signum);
	int (*sys_sigreturn)();
};

int syscall(uint64 a0, uint64 a1, uint64 a2, uint64 a3, uint64 a4, uint64 a5, uint64 a6, uint64 a7);
void set_syscall(struct syscall_context *sys_context);

#endif
