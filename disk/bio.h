#ifndef BUF_H
#define BUF_H

#include "defs.h"
#include "log.h"

/// 实现了磁盘缓存的数据结构buf，一个buf对应了一个磁盘的block
struct buf {
	int valid; ///< has data been read from disk?
	int disk; ///< does disk "own" buf?
	uint dev;
	uint blockno;
	uint refcnt;
	struct buf *prev; ///< LRU cache list
	struct buf *next;
	uchar data[BSIZE];
};

void binit(void);  ///< 初始化一个buf
void* bread(uint, uint);  ///< 读磁盘上block到缓存buf的数据，返回buf的指针。dev是设备编号，blockno是磁盘block的编号
void brelse(void*);  ///< 释放一个buffer
void bwrite(void*); ///< 把buf里面的数据写回磁盘上block，指针指向该buf
void bpin(void*);  ///< 增加一个buf的引用计数
void bunpin(void*);  ///< 减少一个buf的引用计数
uchar* buf_data(void*);  ///< 返回buf存放实际数据位置的地址

#endif // BUF_H
