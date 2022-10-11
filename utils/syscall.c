#include "syscall.h"
#include "syscall_ids.h"
#include "trap.h"

struct syscall_context* sys_context;

void set_syscall(struct syscall_context* syscall_context) {
    sys_context = syscall_context;
}

// syscall函数从上下文中获取对应的syscall id，分别调用对应的syscall处理函数，而对应的syscall处理函数已经在sys_context中指定
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
    }
    trapframe->a0 = ret;
}