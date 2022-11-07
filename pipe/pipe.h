#ifndef PIPE_H
#define PIPE_H

/**
 * @file pipe.h
 * @brief 管道的内部实现
 */

#include "defs.h"
#include "log.h"

#define PIPESIZE (512)

//a struct for pipe
struct pipe {
	char data[PIPESIZE];
	uint nread; ///< number of bytes read
	uint nwrite; ///< number of bytes written
	int readopen; ///< read fd is still open
	int writeopen; ///< write fd is still open
};

void* pipeopen();  ///< 创建一个管道
void pipeclose(void *_pi, int writable);  ///< 关闭一个管道的读/写端。如果writeable为1表示关闭写端，否则表示关闭读端
int pipewrite(void *_pi, uint64 addr, int n);  ///< 往一个管道里面写数据。数据的虚存起始地址为addr，有n个字节
int piperead(void *_pi, uint64 addr, int n);  ///< 从一个管道里面读数据

struct pipe_context
{
	pagetable_t (*get_curr_pagetable)();  ///< 得到当前进程的页表
	void (*yield)();  ///< 写一个满的管道或者读一个空的管道时，需要等待其它进程的操作，所以要交出 CPU 控制权

	/// 管道的数据是存放在物理内存里面的ring buffer，因此需要分配和释放物理内存。这两个接口可以直接让模块分配和释放物理页
	void* (*kalloc)();
	void (*kfree)(void* pa);
	
	/// 管道需要和进程虚拟内存里面的数据进行交互，比如把进程虚存里面的东西写入管道里面，因此需要copyin和copyout接口
	int (*copyin)(pagetable_t pagetable, char* dst, uint64 srcva, uint64 len);
	int (*copyout)(pagetable_t pagetable, uint64 dstva, char* src, uint64 len);
};

void set_pipe(struct pipe_context *pipe_context);  ///< 初始化pipe_os_context

#endif // PIPE_H