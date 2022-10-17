#ifndef TRAP_H
#define TRAP_H

#include "../utils/types.h"

struct trapframe {
	/// kernel page table
	/*   0 */ uint64 kernel_satp;
	/// top of process's kernel stack
	/*   8 */ uint64 kernel_sp;
	/// usertrap()
	/*  16 */ uint64 kernel_trap;
	/// saved user program counter
	/*  24 */ uint64 epc;
	/// saved kernel tp
	/*  32 */ uint64 kernel_hartid;
	/*  40 */ uint64 ra;
	/*  48 */ uint64 sp;
	/*  56 */ uint64 gp;
	/*  64 */ uint64 tp;
	/*  72 */ uint64 t0;
	/*  80 */ uint64 t1;
	/*  88 */ uint64 t2;
	/*  96 */ uint64 s0;
	/* 104 */ uint64 s1;
	/* 112 */ uint64 a0;
	/* 120 */ uint64 a1;
	/* 128 */ uint64 a2;
	/* 136 */ uint64 a3;
	/* 144 */ uint64 a4;
	/* 152 */ uint64 a5;
	/* 160 */ uint64 a6;
	/* 168 */ uint64 a7;
	/* 176 */ uint64 s2;
	/* 184 */ uint64 s3;
	/* 192 */ uint64 s4;
	/* 200 */ uint64 s5;
	/* 208 */ uint64 s6;
	/* 216 */ uint64 s7;
	/* 224 */ uint64 s8;
	/* 232 */ uint64 s9;
	/* 240 */ uint64 s10;
	/* 248 */ uint64 s11;
	/* 256 */ uint64 t3;
	/* 264 */ uint64 t4;
	/* 272 */ uint64 t5;
	/* 280 */ uint64 t6;
};

enum Exception {
	InstructionMisaligned = 0,
	InstructionAccessFault = 1,
	IllegalInstruction = 2,
	Breakpoint = 3,
	LoadMisaligned = 4,
	LoadAccessFault = 5,
	StoreMisaligned = 6,
	StoreAccessFault = 7,
	UserEnvCall = 8,
	SupervisorEnvCall = 9,
	MachineEnvCall = 11,
	InstructionPageFault = 12,
	LoadPageFault = 13,
	StorePageFault = 15,
};

enum Interrupt {
	UserSoft = 0,
	SupervisorSoft,
	UserTimer = 4,
	SupervisorTimer,
	UserExternal = 8,
	SupervisorExternal,
};

struct trap_handler_context
{
	/// yield是每个章节自己的，不同的调度策略会调用不同的sched，而ch2里面甚至不需要支持（空函数）
	void (*yield)();
	
	/// 主要用于处理外部中断的时候，获取当前是哪个cpu（发生了外部中断）
	int (*cpuid)();

	/// set_usertrap和set_kerneltrap分别用于处理在U/S态下发生异常时的异常处理代码入口
	void (*set_usertrap)();
	void (*set_kerneltrap)();
	
	/// 用于得到当前正在考虑的trapframe,在ch2里面是全局共用的，到后面就是不同进程、再到不同线程的trap_frame
	struct trapframe* (*get_trapframe)();
	/// 用于拿到内核栈的栈顶指针
	uint64 (*get_kernel_sp)();
	
	/// 放在usertrapret的最后执行，是在从S态回到U态前要做的一些工作，如把页表地址取出，然后调用userret最终回到S态
	void (*call_userret)();
	/// 在usertrap执行结束后执行，cause是异常编号，这里主要是为了兼容ch2，后面的章节里面只要调用usertrapret即可
	void (*finish_usertrap)(int cause);
	/// 用于处理当trap处理发生错误时要做的事
	void (*error_in_trap)(int status);

	/// 用于处理非时间中断的一切外部中断，会在文件系统里面用到
	void (*super_external_handler)(int cpuid);
};

void set_trap(struct trap_handler_context *trap_handler_context);
void usertrapret();
void kerneltrap();

#endif // TRAP_H