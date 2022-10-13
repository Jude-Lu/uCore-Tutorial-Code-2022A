#include "os6_syscall.h"
#include "../utils/defs.h"
#include "loader.h"
#include "os6_trap.h"

uint64 console_write(uint64 va, uint64 len)
{
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	int size = copyinstr(p->pagetable, str, va, MIN(len, MAX_STR_LEN));
	tracef("write size = %d", size);
	for (int i = 0; i < size; ++i) {
		console_putchar(str[i]);
	}
	return len;
}

uint64 console_read(uint64 va, uint64 len)
{
	struct proc *p = curr_proc();
	char str[MAX_STR_LEN];
	tracef("read size = %d", len);
	for (int i = 0; i < len; ++i) {
		int c = consgetc();
		str[i] = c;
	}
	copyout(p->pagetable, va, str, len);
	return len;
}

uint64 os6_sys_write(int fd, uint64 va, uint64 len)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_proc();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d\n", fd);
		return -1;
	}
	switch (f->type) {
	case FD_STDIO:
		return console_write(va, len);
	case FD_INODE:
		return inodewrite(f, va, len);
	default:
		panic("unknown file type %d\n", f->type);
		__builtin_unreachable();
	}
}

uint64 os6_sys_read(int fd, uint64 va, uint64 len)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_proc();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d\n", fd);
		return -1;
	}
	switch (f->type) {
	case FD_STDIO:
		return console_read(va, len);
	case FD_INODE:
		return inoderead(f, va, len);
	default:
		panic("unknown file type %d\n", f->type);
		__builtin_unreachable();
	}
}

__attribute__((noreturn)) void os6_sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 os6_sys_sched_yield()
{
	yield();
	return 0;
}

uint64 os6_sys_gettimeofday(uint64 val, int _tz)
{
	struct proc *p = curr_proc();
	uint64 cycle = get_cycle();
	TimeVal t;
	t.sec = cycle / CPU_FREQ;
	t.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, val, (char *)&t, sizeof(TimeVal));
	return 0;
}

uint64 os6_sys_getpid()
{
	return curr_proc()->pid;
}

uint64 os6_sys_getppid()
{
	struct proc *p = curr_proc();
	return p->parent == NULL ? IDLE_PID : p->parent->pid;
}

uint64 os6_sys_clone()
{
	debugf("fork!");
	return fork();
}

static inline uint64 fetchaddr(pagetable_t pagetable, uint64 va)
{
	uint64 *addr = (uint64 *)useraddr(pagetable, va);
	return *addr;
}

uint64 os6_sys_exec(uint64 path, uint64 uargv)
{
	struct proc *p = curr_proc();
	char name[MAX_STR_LEN];
	copyinstr(p->pagetable, name, path, MAX_STR_LEN);
	uint64 arg;
	static char strpool[MAX_ARG_NUM][MAX_STR_LEN];
	char *argv[MAX_ARG_NUM];
	int i;
	for (i = 0; uargv && (arg = fetchaddr(p->pagetable, uargv));
	     uargv += sizeof(char *), i++) {
		copyinstr(p->pagetable, (char *)strpool[i], arg, MAX_STR_LEN);
		argv[i] = (char *)strpool[i];
	}
	argv[i] = NULL;
	return exec(name, (char **)argv);
}

uint64 os6_sys_wait(int pid, uint64 va)
{
	struct proc *p = curr_proc();
	int *code = (int *)useraddr(p->pagetable, va);
	return wait(pid, code);
}

uint64 os6_sys_spawn(uint64 va)
{
	// TODO: your job is to complete the sys call
	return -1;
}

uint64 os6_sys_set_priority(long long prio)
{
	// TODO: your job is to complete the sys call
	return -1;
}

uint64 os6_sys_openat(uint64 va, uint64 omode, uint64 _flags)
{
	struct proc *p = curr_proc();
	char path[200];
	copyinstr(p->pagetable, path, va, 200);
	return fileopen(path, omode);
}

uint64 os6_sys_close(int fd)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_proc();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d", fd);
		return -1;
	}
	fileclose(f);
	p->files[fd] = 0;
	return 0;
}

int os6_sys_fstat(int fd,uint64 stat){
	//TODO: your job is to complete the syscall
	return -1;
}

int os6_sys_linkat(int olddirfd, uint64 oldpath, int newdirfd, uint64 newpath, uint64 flags){
	//TODO: your job is to complete the syscall
	return -1;
}

int os6_sys_unlinkat(int dirfd, uint64 name, uint64 flags){
	//TODO: your job is to complete the syscall
	return -1;
}

void syscall_init()
{
    static struct syscall_context os6_sys_context =
    {
        .sys_write = os6_sys_write,
		.sys_read = os6_sys_read,
        .sys_exit = os6_sys_exit,
		.sys_sched_yield = os6_sys_sched_yield,
        .sys_gettimeofday = os6_sys_gettimeofday,
		.sys_getpid = os6_sys_getpid,
		.sys_getppid = os6_sys_getppid,
		.sys_clone = os6_sys_clone,
		.sys_exec = os6_sys_exec,
		.sys_wait = os6_sys_wait,
		.sys_spawn = os6_sys_spawn,
		.sys_set_priority = os6_sys_set_priority,
		.sys_openat = os6_sys_openat,
		.sys_close = os6_sys_close,
		.sys_fstat = os6_sys_fstat,
		.sys_linkat = os6_sys_linkat,
		.sys_unlinkat = os6_sys_unlinkat
	};
    set_syscall(&os6_sys_context);
}
