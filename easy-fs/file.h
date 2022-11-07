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

/**
 * inode 指的是Index Node（索引节点），是文件系统中的一种重要数据结构。\n
 * 逻辑目录树结构中的每个文件和目录都对应一个 inode。\n
 * 在 inode 中不仅包含了文件/目录的元数据（大小/访问权限/类型等信息），\n
 * 还包含实际保存对应文件/目录数据的数据块（位于最后的数据块区域中）的索引信息，\n
 * 从而能够找到文件/目录的数据被保存在磁盘的哪些块中。
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
/// 在进程中，我们使用file结构体来标识一个被进程使用的文件
/// Defines a file in memory that provides information about the current use of the file and the corresponding inode location
struct file {
	enum { FD_NONE = 0, FD_PIPE, FD_INODE, FD_STDIO } type;
	int ref; ///< reference count
	char readable;
	char writable;
	void *pipe; ///< FD_PIPE
	struct inode *ip; ///< FD_INODE
	uint off; ///< 读写文件的偏移量；可以使用lseek函数来改变这个偏移的位置以支持随机读写
};

void fileclose(struct file *);  ///< 关闭文件f
int pipealloc(struct file *, struct file *);  ///< 新建一个pipe，读端和写端对用户暴露接口的文件分别为f1和f2
int fileopen(char *, uint64);  ///< 打开路径为path的文件；如果omode为O_CREATE需要新建一个文件，否则打开一个已有的文件
uint64 inodewrite(struct file *, uint64, uint64);  ///< 向文件写数据，写入数据的起始虚拟地址是va、数据的字节长度为len，返回成功写入的数据长度。这里取名为inode 是因为，easy-fs实际上是对文件被绑定的inode写数据的；考虑同时打开了多次同一个文件（假设不关闭）的情形，这时候进程里面会有多个fd（以及对应的 file 结构体，它们会有不同的off表示读写的偏移量）对应同一个inode
uint64 inoderead(struct file *, uint64, uint64);  ///< 从文件读数据
struct file *stdio_init(int);  ///< 初始化一个file类型的结构体
int show_all_files();

struct FSManager
{
	/// 文件对于进程而言也是其需要记录的一种资源，每个打开的文件会有一个非负整数 fd 指代它，这是一种已经被打开文件的索引。该接口用于让当前进程分配一个新的fd。
	int (*fdalloc)(struct file *f);
	/// 分配进程中实际存储 struct file 指针的资源
	struct file* (*filealloc)();

	/// 以下三个接口用于和当前进程的虚存做交互
	pagetable_t (*get_curr_pagetable)();
	int (*either_copyout)(pagetable_t, int, uint64, char*, uint64);
	int (*either_copyin)(pagetable_t, int, uint64, char*, uint64);

	/// 对于用户来说，pipe被抽象为了文件，也就是说文件系统要提供操作pipe的接口。实际的实现文件系统不负责，需要内核核心提供的创建和关闭管道的接口；内核核心可以直接使用pipe模块提供的接口
	void* (*pipeopen)();
	void (*pipeclose)(void *_pi, int writable);

	/// 以下为与磁盘交互的接口。内核核心可以把disk模块对外暴露的接口提供给easy-fs模块使用
	void* (*bread)(uint, uint);
	void (*brelse)(void*);
	void (*bwrite)(void*);
	void (*bpin)(void*);
	void (*bunpin)(void*);
	uchar* (*buf_data)(void*);
};

void set_file(struct FSManager *FSManager);  ///< 设置fs_manager

#endif // FILE_H