#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "defs.h"

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

enum threadstate { T_UNUSED, T_USED, T_SLEEPING, T_RUNNABLE, T_RUNNING, T_EXITED };

// Saved registers for kernel context switches.
struct context {
	uint64 ra;
	uint64 sp;

	// callee-saved
	uint64 s0;
	uint64 s1;
	uint64 s2;
	uint64 s3;
	uint64 s4;
	uint64 s5;
	uint64 s6;
	uint64 s7;
	uint64 s8;
	uint64 s9;
	uint64 s10;
	uint64 s11;
};

/// 注意返回类型为void*的接口，实际返回类型就是proc结构体指针，但proc在不同的lab中定义不同，因此采用无类型指针定义
struct manager {
	/// 创建任务
	void* (*create)();
	/// 删除任务
	void (*remove)(void* p);
	/// 根据id获取任务
	void* (*get_task_by_id)(int id);
	/// 根据任务获取id
	int (*get_id_by_task)(void* p);
	/// 将任务加入调度队列
	void (*add)(void* p);
	/// 从调度队列中取出相应任务
	void* (*fetch)();
};

void set_manager(struct manager*);
void* curr_task();
void set_curr(void*);
void* get_task(int);
int get_id(void*);
void* alloc_task();
void free_task(void*);
void add_task(void*);
void* fetch_task();

#endif // PROCESSOR_H
