#include "os7_syscall.h"
#include "loader.h"
#include "os7_trap.h"

uint64 console_write(uint64 va, uint64 len)
{
	struct proc *p = curr_task();
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
	struct proc *p = curr_task();
	char str[MAX_STR_LEN];
	tracef("read size = %d", len);
	for (int i = 0; i < len; ++i) {
		int c = consgetc();
		str[i] = c;
	}
	copyout(p->pagetable, va, str, len);
	return len;
}

uint64 os7_sys_write(int fd, uint64 va, uint64 len)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_task();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d\n", fd);
		return -1;
	}
	switch (f->type) {
	case FD_STDIO:
		return console_write(va, len);
	case FD_PIPE:
		return pipewrite(f->pipe, va, len);
	case FD_INODE:
		return inodewrite(f, va, len);
	default:
		panic("unknown file type %d\n", f->type);
		__builtin_unreachable();
	}
}

uint64 os7_sys_read(int fd, uint64 va, uint64 len)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_task();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d\n", fd);
		return -1;
	}
	switch (f->type) {
	case FD_STDIO:
		return console_read(va, len);
	case FD_PIPE:
		return piperead(f->pipe, va, len);
	case FD_INODE:
		return inoderead(f, va, len);
	default:
		panic("unknown file type %d\n", f->type);
		__builtin_unreachable();
	}
}

__attribute__((noreturn)) void os7_sys_exit(int code)
{
	exit(code);
	__builtin_unreachable();
}

uint64 os7_sys_sched_yield()
{
	yield();
	return 0;
}

uint64 os7_sys_gettimeofday(uint64 val, int _tz)
{
	struct proc *p = curr_task();
	uint64 cycle = get_cycle();
	TimeVal t;
	t.sec = cycle / CPU_FREQ;
	t.usec = (cycle % CPU_FREQ) * 1000000 / CPU_FREQ;
	copyout(p->pagetable, val, (char *)&t, sizeof(TimeVal));
	return 0;
}

uint64 os7_sys_getpid()
{
	return ((struct proc*)curr_task())->pid;
}

uint64 os7_sys_getppid()
{
	struct proc *p = curr_task();
	return p->parent == NULL ? IDLE_PID : p->parent->pid;
}

uint64 os7_sys_clone()
{
	debugf("fork!");
	return fork();
}

static inline uint64 fetchaddr(pagetable_t pagetable, uint64 va)
{
	uint64 *addr = (uint64 *)useraddr(pagetable, va);
	return *addr;
}

uint64 os7_sys_exec(uint64 path, uint64 uargv)
{
	struct proc *p = curr_task();
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

uint64 os7_sys_wait(int pid, uint64 va)
{
	struct proc *p = curr_task();
	int *code = (int *)useraddr(p->pagetable, va);
	return wait(pid, code);
}

uint64 os7_sys_pipe(uint64 fdarray)
{
	struct proc *p = curr_task();
	uint64 fd0, fd1;
	struct file *f0, *f1;
	if (f0 < 0 || f1 < 0) {
		return -1;
	}
	f0 = filealloc();
	f1 = filealloc();
	if (pipealloc(f0, f1) < 0)
		goto err0;
	fd0 = fdalloc(f0);
	fd1 = fdalloc(f1);
	if (fd0 < 0 || fd1 < 0)
		goto err0;
	if (copyout(p->pagetable, fdarray, (char *)&fd0, sizeof(fd0)) < 0 ||
	    copyout(p->pagetable, fdarray + sizeof(uint64), (char *)&fd1,
		    sizeof(fd1)) < 0) {
		goto err1;
	}
	return 0;

err1:
	p->files[fd0] = 0;
	p->files[fd1] = 0;
err0:
	fileclose(f0);
	fileclose(f1);
	return -1;
}

uint64 os7_sys_openat(uint64 va, uint64 omode, uint64 _flags)
{
	struct proc *p = curr_task();
	char path[200];
	copyinstr(p->pagetable, path, va, 200);
	return fileopen(path, omode);
}

uint64 os7_sys_close(int fd)
{
	if (fd < 0 || fd > FD_BUFFER_SIZE)
		return -1;
	struct proc *p = curr_task();
	struct file *f = p->files[fd];
	if (f == NULL) {
		errorf("invalid fd %d", fd);
		return -1;
	}
	fileclose(f);
	p->files[fd] = 0;
	return 0;
}

void syscall_init()
{
	static struct syscall_context os7_sys_context =
	{
		.sys_write = os7_sys_write,
		.sys_read = os7_sys_read,
		.sys_exit = os7_sys_exit,
		.sys_sched_yield = os7_sys_sched_yield,
		.sys_gettimeofday = os7_sys_gettimeofday,
		.sys_getpid = os7_sys_getpid,
		.sys_getppid = os7_sys_getppid,
		.sys_clone = os7_sys_clone,
		.sys_exec = os7_sys_exec,
		.sys_wait = os7_sys_wait,
		.sys_pipe = os7_sys_pipe,
		.sys_openat = os7_sys_openat,
		.sys_close = os7_sys_close
	};
	set_syscall(&os7_sys_context);
}
