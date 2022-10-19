#ifndef CONST_H
#define CONST_H

#include "riscv.h"

#define PAGE_SIZE (0x1000)

enum {
	STDIN = 0,
	STDOUT = 1,
	STDERR = 2,
};

// memory layout

// the kernel expects there to be RAM
// for use by the kernel and user pages
// from physical address 0x80000000 to PHYSTOP.
#define KERNBASE 0x80200000L
#define PHYSTOP (0x80000000 + 128 * 1024 * 1024) // we have 128M memroy

// one beyond the highest possible virtual address.
// MAXVA is actually one bit less than the max allowed by
// Sv39, to avoid having to sign-extend virtual addresses
// that have the high bit set.
#define MAXVA (1L << (9 + 9 + 9 + 12 - 1))

// map the trampoline page to the highest address,
// in both user and kernel space.
#define USER_TOP (MAXVA)
#define TRAMPOLINE (USER_TOP - PGSIZE)
#define TRAPFRAME (TRAMPOLINE - PGSIZE)

#define MAX_APP_NUM (32)
#define MAX_STR_LEN (300)
#define IDLE_PID (0)
#define MAX_ARG_NUM (32) // max exec arguments


// file system
#define NFILE (100) // open files per system
#define NINODE (50) // maximum number of active i-nodes
#define NDEV (10) // maximum major device number
#define ROOTDEV (1) // device number of file system root disk
#define MAXOPBLOCKS (10) // max # of blocks any FS op writes
#define NBUF (MAXOPBLOCKS * 3) // size of disk block cache
#define FSSIZE (1000) // size of file system in blocks
#define MAXPATH (128) // maximum file path name

#define ROOTINO (1) // root i-number
#define BSIZE (1024) // block size

// virtio mmio interface
#define VIRTIO0 0x10001000
#define VIRTIO0_IRQ 1

#endif // CONST_H
