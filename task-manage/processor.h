#ifndef PROCESSOR_H
#define PROCESSOR_H

/**
 * @file processor.h
 * @brief 主要负责任务的调度
*/

#include "defs.h"

enum procstate { UNUSED, USED, SLEEPING, RUNNABLE, RUNNING, ZOMBIE };

enum threadstate { T_UNUSED, T_USED, T_SLEEPING, T_RUNNABLE, T_RUNNING, T_EXITED };

/// Saved registers for kernel context switches.
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

/// manager各个接口的实现在各个lab中，在这里初始化proc_manager
void set_manager(struct manager*);
/// 获得当前正在运行的进程
void* curr_task();
/// 修改当前正在运行的进程
void set_curr(void*);
/// 根据id获得进程
void* get_task(int);
/// 根据进程获得id
int get_id(void*);
/// 创建进程
void* alloc_task();
/// 释放进程
void free_task(void*);
/// 阻塞进程
void add_task(void*);
/// 调度进程
void* fetch_task();

#endif // PROCESSOR_H
