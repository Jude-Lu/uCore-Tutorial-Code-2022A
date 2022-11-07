#ifndef SYNC_H
#define SYNC_H

/**
 * @file sync.h
 * @brief 同步互斥模块，实现了mutex（锁）、semaphore（信号量）、condvar（条件变量）三种机制
 */

#include "defs.h"
#include "sync_dependency.h"

#define WAIT_QUEUE_MAX_LENGTH 16

struct mutex {
	uint blocking;
	uint locked;
	struct queue wait_queue;
	// "alloc" data for wait queue
	int _wait_queue_data[WAIT_QUEUE_MAX_LENGTH];
};

struct semaphore {
	int count;
	struct queue wait_queue;
	// "alloc" data for wait queue
	int _wait_queue_data[WAIT_QUEUE_MAX_LENGTH];
};

struct condvar {
	struct queue wait_queue;
	// "alloc" data for wait queue
	int _wait_queue_data[WAIT_QUEUE_MAX_LENGTH];
};

struct mutex *mutex_create(int blocking);  ///< 创建一个mutex，其中blocking表示mutex的类型，为0是自旋锁、否则是阻塞锁
void mutex_lock(struct mutex *);  ///< 加锁
void mutex_unlock(struct mutex *);  ///< 解锁
struct semaphore *semaphore_create(int count);  ///< 创建一个初值为count的semaphore
void semaphore_up(struct semaphore *);  ///< 将semaphore的值加1
void semaphore_down(struct semaphore *);  ///< 将semaphore的值减1
struct condvar *condvar_create();  ///< 创建一个condvar
void cond_signal(struct condvar *);  ///< 在该condvar上发信号，唤醒正在等待的线程
void cond_wait(struct condvar *, struct mutex *);  ///< 线程使用该接口来等到一个condvar为真（即有其它线程发信号）

struct synchronization_context
{
	/// mutex、semaphore、condvar是进程掌握的资源，因此需要让内核核心实现分配的功能
	struct mutex* (*alloc_mutex)();
	struct semaphore* (*alloc_semaphore)();
	struct condvar* (*alloc_codvar)();

	int (*curr_task_id)();  ///< 获取当前正在运行线程的id。内核核心需要给每个线程提供互不相同的int类型id，以供同步互斥模块识别

	void (*yield)();  ///< 对于spin mutex（自旋锁）的机制，线程抢不到锁需要等待，这时候需要主动放弃 CPU 主动权

	void (*sleeping)();  ///< 让当前抢不到资源的线程睡眠，等到后面资源空闲时被唤醒
	void (*running)(int id);  ///< 唤醒编号为id的线程
};

void set_sync(struct synchronization_context *synchronization_context);  ///< 初始化sync_context


#endif
