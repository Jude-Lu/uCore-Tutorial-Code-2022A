#ifndef FILE_H
#define FILE_H

#include "fs.h"
#include "defs.h"

/**
 * @file file.h
 * @brief Inode content
 * @details
 * The content (data) associated with each inode is stored \n
 * in blocks on the disk. The first NDIRECT block numbers \n
 * are listed in ip->addrs[].  The next NINDIRECT blocks are \n
 * listed in block ip->addrs[NDIRECT]. \n
 * 
 * in-memory copy of an inode,it can be used to quickly locate file entities on disk
 */
struct inode {
	uint dev; ///< Device number
	uint inum; ///< Inode number
	int ref; ///< Reference count
	int valid; ///< inode has been read from disk?
	short type; ///< copy of disk inode
	uint size;
	uint addrs[NDIRECT + 1];
	// LAB4: You may need to add link count here
};

/// file.h
/// Defines a file in memory that provides information about the current use of the file and the corresponding inode location
struct file {
	enum { FD_NONE = 0, FD_PIPE, FD_INODE, FD_STDIO } type;
	int ref; ///< reference count
	char readable;
	char writable;
	void *pipe; ///< FD_PIPE
	struct inode *ip; ///< FD_INODE
	uint off;
};

void fileclose(struct file *);
int pipealloc(struct file *, struct file *);
int fileopen(char *, uint64);
uint64 inodewrite(struct file *, uint64, uint64);
uint64 inoderead(struct file *, uint64, uint64);
struct file *stdio_init(int);
int show_all_files();

struct FSManager
{
	int (*fdalloc)(struct file *f);
	struct file* (*filealloc)();
	pagetable_t (*get_curr_pagetable)();
	
	int (*either_copyout)(pagetable_t, int, uint64, char*, uint64);
	int (*either_copyin)(pagetable_t, int, uint64, char*, uint64);

	void* (*pipeopen)();
	void (*pipeclose)(void *_pi, int writable);

	void* (*bread)(uint, uint);
	void (*brelse)(void*);
	void (*bwrite)(void*);
	void (*bpin)(void*);
	void (*bunpin)(void*);
	uchar* (*buf_data)(void*);
};

void set_file(struct FSManager *FSManager);

#endif // FILE_H