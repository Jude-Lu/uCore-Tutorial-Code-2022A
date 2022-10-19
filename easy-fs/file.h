#ifndef FILE_H
#define FILE_H

#include "fs.h"
#include "../utils/types.h"
#include "../utils/riscv.h"

// in-memory copy of an inode,it can be used to quickly locate file entities on disk
struct inode {
	uint dev; // Device number
	uint inum; // Inode number
	int ref; // Reference count
	int valid; // inode has been read from disk?
	short type; // copy of disk inode
	uint size;
	uint addrs[NDIRECT + 1];
	// LAB4: You may need to add link count here
};

// file.h
// Defines a file in memory that provides information about the current use of the file and the corresponding inode location
struct file {
	enum { FD_NONE = 0, FD_PIPE, FD_INODE, FD_STDIO } type;
	int ref; // reference count
	char readable;
	char writable;
	struct pipe *pipe; // FD_PIPE
	struct inode *ip; // FD_INODE
	uint off;
};

void fileclose(struct file *);
struct file *filealloc();
int fileopen(char *, uint64);
uint64 inodewrite(struct file *, uint64, uint64);
uint64 inoderead(struct file *, uint64, uint64);
struct file *stdio_init(int);
int show_all_files();

struct FSManager
{
	int filepool_size;
	struct file *filepool; // This is a system-level open file table that holds open files of all process.

	int (*fdalloc)(struct file *f);
	pagetable_t (*get_curr_pagetable)();
	
	int (*either_copyout)(pagetable_t, int, uint64, char*, uint64);
	int (*either_copyin)(pagetable_t, int, uint64, char*, uint64);

	void (*pipeclose)(void *_pi, int writable);
};

void set_file(struct FSManager *FSManager);

#endif // FILE_H