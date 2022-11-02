#ifndef QUEUE_H
#define QUEUE_H
#define QUEUE_SIZE (1024)

/**
 * @file queue.h
 * @brief 实现了简单的循环队列，可以在进程调度和同步互斥中使用
*/

/// queue data for processing scheduling only
/// for queue for wait queue of mutex/semaphore/condvar, provide other data
extern int process_queue_data[QUEUE_SIZE];

struct queue {
	int *data;
	int size;
	int front;
	int tail;
	int empty;
};

void init_queue(struct queue *, int, int *); ///< 队列初始化
void push_queue(struct queue *, int); ///< 进队
int pop_queue(struct queue *); ///< 出队
int is_empty(struct queue *); ///< 队列是否为空

#endif // QUEUE_H
