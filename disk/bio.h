#ifndef BUF_H
#define BUF_H

#include "../utils/types.h"
#include "../utils/const.h"

struct buf {
	int valid; // has data been read from disk?
	int disk; // does disk "own" buf?
	uint dev;
	uint blockno;
	uint refcnt;
	struct buf *prev; // LRU cache list
	struct buf *next;
	uchar data[BSIZE];
};

void binit(void);
void* bread(uint, uint);
void brelse(void*);
void bwrite(void*);
void bpin(void*);
void bunpin(void*);
uchar* buf_data(void*);

#endif // BUF_H
