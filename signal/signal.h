#ifndef SIGNAL_H
#define SIGNAL_H

#include "defs.h"
#include "signal_dependency.h"

struct sigaction {
	void (*handler)(int);
	uint64   sa_mask;
};

struct signal_block {
	uint32 signals; // 要响应的信号
	uint32 signal_mask; // 要屏蔽的信号
	uint32 handling_sig; // 正在处理的信号
	int killed, frozen;
	struct sigaction sig_actions[32];
};

struct signal_context {
	pagetable_t (*get_curr_pagetable)();  ///< 得到当前进程的页表

	struct signal_block* (*get_curr_sig_block)(); //< 得到当前进程的signal_block
	struct signal_block* (*pid2sig_block)(int pid);
	
	void (*recover_sig_trapframe)();

	int (*copyin)(pagetable_t pagetable, char* dst, uint64 srcva, uint64 len);
	int (*copyout)(pagetable_t pagetable, uint64 dstva, char* src, uint64 len);
};
void set_signal(struct signal_context*);

int sigaction(uint32, uint64, uint64);
int sigprocmask(uint32);
int sigreturn();
int sigkill(int, uint32);

#define SIGHUP 1
#define SIGINT 2
#define SIGQUIT 3
#define SIGILL 4
#define SIGTRAP 5
#define SIGABRT 6
#define SIGBUS 7
#define SIGFPE 8
#define SIGKILL 9
#define SIGUSR1 10
#define SIGSEGV 11
#define SIGUSR2 12
#define SIGPIPE 13
#define SIGALRM 14
#define SIGTERM 15
#define SIGSTKFLT 16
#define SIGCHLD 17
#define SIGCONT 18
#define SIGSTOP 19
#define SIGTSTP 20
#define SIGTTIN 21
#define SIGTTOU 22
#define SIGURG 23
#define SIGXCPU 24
#define SIGXFSZ 25
#define SIGVTALRM 26
#define SIGPROF 27
#define SIGWINCH 28
#define SIGIO 29
#define SIGPWR 30
#define SIGSYS 31

#endif