/**
 * @file bio.c
 * @brief Buffer cache.
 * 
 * @details
 * The buffer cache is a linked list of buf structures holding \n
 * cached copies of disk block contents.  Caching disk blocks \n
 * in memory reduces the number of disk reads and also provides \n
 * a synchronization point for disk blocks used by multiple processes. \n
 * 
 * Interface: \n
 * * To get a buffer for a particular disk block, call bread. \n
 * * After changing buffer data, call bwrite to write it to disk. \n
 * * When done with the buffer, call brelse. \n
 * * Do not use the buffer after calling brelse. \n
 * * Only one process at a time can use a buffer, \n
 *     so do not keep them longer than necessary.
 */

#include "bio.h"
#include "virtio.h"

struct {
	struct buf buf[NBUF];
	struct buf head;
} bcache;

void binit()
{
	struct buf *b;
	// Create linked list of buffers
	bcache.head.prev = &bcache.head;
	bcache.head.next = &bcache.head;
	for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
		b->next = bcache.head.next;
		b->prev = &bcache.head;
		bcache.head.next->prev = b;
		bcache.head.next = b;
	}
}

/// Look through buffer cache for block on device dev.
/// If not found, allocate a buffer.
static struct buf *bget(uint dev, uint blockno)
{
	struct buf *b;
	// Is the block already cached?
	for (b = bcache.head.next; b != &bcache.head; b = b->next) {
		if (b->dev == dev && b->blockno == blockno) {
			b->refcnt++;
			return b;
		}
	}
	// Not cached.
	// Recycle the least recently used (LRU) unused buffer.
	for (b = bcache.head.prev; b != &bcache.head; b = b->prev) {
		if (b->refcnt == 0) {
			b->dev = dev;
			b->blockno = blockno;
			b->valid = 0;
			b->refcnt = 1;
			return b;
		}
	}
	panic("bget: no buffers");
	return 0;
}

const int R = 0;
const int W = 1;

/// Return a buf with the contents of the indicated block.
void* bread(uint dev, uint blockno)
{
	struct buf *b;
	b = bget(dev, blockno);
	if (!b->valid) {
		virtio_disk_rw(b, R);
		b->valid = 1;
	}
	return (void*)b;
}

/// Write b's contents to disk.
void bwrite(void *_b)
{
	struct buf *b = _b;
	virtio_disk_rw(b, W);
}

/// Release a buffer.
/// Move to the head of the most-recently-used list.
void brelse(void *_b)
{
	struct buf *b = _b;
	b->refcnt--;
	if (b->refcnt == 0) {
		// no one is waiting for it.
		b->next->prev = b->prev;
		b->prev->next = b->next;
		b->next = bcache.head.next;
		b->prev = &bcache.head;
		bcache.head.next->prev = b;
		bcache.head.next = b;
	}
}

void bpin(void *b)
{
	((struct buf*)b)->refcnt++;
}

void bunpin(void *b)
{
	((struct buf*)b)->refcnt--;
}

uchar* buf_data(void *b)
{
	return ((struct buf*)b) -> data;
}