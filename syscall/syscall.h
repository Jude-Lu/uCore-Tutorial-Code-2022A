#ifndef SYSCALL_H
#define SYSCALL_H

#include "../utils/types.h"
#include "../trap/trap.h"

//syscall_context定义了各个syscall的函数指针，方便兼容不同章节syscall的差异，各个章节可以根据本章节的内容和难度实现本章节的syscall
struct syscall_context{
	//ch2 syscall
	uint64 (*sys_write)(int fd, uint64 va, uint64 len);
	void (*sys_exit)(int code);

	//ch3 新增syscall
	uint64 (*sys_sched_yield)();
	uint64 (*sys_gettimeofday)(uint64 val, int _tz);

	//ch5 新增syscall
	uint64 (*sys_read)(int fd, uint64 va, uint64 len);
	uint64 (*sys_getpid)();
	uint64 (*sys_getppid)();
	uint64 (*sys_clone)();
	uint64 (*sys_exec)(uint64 path, uint64 uargv);
	uint64 (*sys_wait)(int pid, uint64 va);
	uint64 (*sys_spawn)(uint64 va);//该syscall需学生在实验中实现
	uint64 (*sys_set_priority)(long long prio);//该syscall需学生在实验中实现

	//ch6 新增syscall
	uint64 (*sys_openat)(uint64 va, uint64 omode, uint64 _flags);
	uint64 (*sys_close)(int fd);
	int (*sys_fstat)(int fd,uint64 stat);//该syscall需学生在实验中实现
	int (*sys_linkat)(int olddirfd, uint64 oldpath, int newdirfd, uint64 newpath, uint64 flags);//该syscall需学生在实验中实现
	int (*sys_unlinkat)(int dirfd, uint64 name, uint64 flags);//该syscall需学生在实验中实现

	//ch7 新增syscall
	uint64 (*sys_pipe)(uint64 fdarray);
};

void syscall(struct trapframe *trapframe);
void set_syscall(struct syscall_context *sys_context);

#endif