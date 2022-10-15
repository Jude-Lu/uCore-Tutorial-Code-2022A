# syscall
## 结构体
```c
//syscall_context定义了各个syscall的函数指针，方便兼容不同章节syscall的差异，各个章节可以根据本章接的内容和难度实现本章节的syscall
struct syscall_context{
    //ch2 syscall
	uint64 (*sys_write)(int fd, uint64 va, uint64 len);
	void (*sys_exit)(int code);

	//ch3 新增syscall
	uint64 (*sys_sched_yield)();
	uint64 (*sys_gettimeofday)(uint64 val, int _tz);

	//ch5 新增syscall
	uint64 (*sys_read)(int fd, uint64 va, uint64 len);
	uint64 (*sys_getpid)();
	uint64 (*sys_getppid)();
	uint64 (*sys_clone)();
	uint64 (*sys_exec)(uint64 path, uint64 uargv);
	uint64 (*sys_wait)(int pid, uint64 va);
	uint64 (*sys_spawn)(uint64 va);//该syscall需学生在实验中实现
	uint64 (*sys_set_priority)(long long prio);//该syscall需学生在实验中实现

	//ch6 新增syscall
	uint64 (*sys_openat)(uint64 va, uint64 omode, uint64 _flags);
	uint64 (*sys_close)(int fd);
	int (*sys_fstat)(int fd,uint64 stat);//该syscall需学生在实验中实现
	int (*sys_linkat)(int olddirfd, uint64 oldpath, int newdirfd, uint64 newpath, uint64 flags);//该syscall需学生在实验中实现
	int (*sys_unlinkat)(int dirfd, uint64 name, uint64 flags);//该syscall需学生在实验中实现

	//ch7 新增syscall
	uint64 (*sys_pipe)(uint64 fdarray);

    //ch8 新增syscall
	int (*sys_thread_create)(uint64 entry, uint64 arg);
	int (*sys_gettid)();
	int (*sys_waittid)(int tid);
	int (*sys_mutex_create)(int blocking);
	int (*sys_mutex_lock)(int mutex_id);
	int (*sys_mutex_unlock)(int mutex_id);
	int (*sys_semaphore_create)(int res_count);
	int (*sys_semaphore_up)(int semaphore_id);
	int (*sys_semaphore_down)(int semaphore_id);
	int (*sys_condvar_create)();
	int (*sys_condvar_signal)(int cond_id);
	int (*sys_condvar_wait)(int cond_id, int mutex_id);
}
```
## 函数
```c
struct syscall_context* sys_context;

// 初始化sys_context
void set_syscall(struct syscall_context* syscall_context) {
	sys_context = syscall_context;
}

// syscall函数从上下文中获取对应的syscall id，分别调用对应的syscall处理函数，而对应的syscall处理函数已经在sys_context中指定
void syscall(struct trapframe* trapframe) {
	int id = trapframe->a7, ret = 0;
	uint64 args[6] = {trapframe->a0, trapframe->a1, trapframe->a2, trapframe->a3, trapframe->a4, trapframe->a5};
	switch (id) {
         case SYS_write:
             ret = sys_context->sys_write(args[0], args[1], args[2]);
             break;
         case SYS_read:
             ret = sys_contex->sys_read(args[0], args[1], args[2]);
             break;
         //省略其他syscall对应的处理代码
         //...
     }
}
```
```c
//实现具体的sys_write函数
uint64 sys_write(int fd, uint64 va, uint64 len){
    //省略具体实现代码
    //...
}

//在调用syscall()之前
void syscall_init()
{
	static struct syscall_context os_sys_context =
	{
		.sys_write = sys_write,
		.sys_read = sys_read,
		//省略指向其他syscall的代码
    	//...
	};
	set_syscall(&os_sys_context);
}
```
# trap
> 仿照syscall_context，可以做一个trap_handler_context，核心思想是把处理trap里面跟系统具体实现强相关的部分拿出来，定义如下：

```c
struct trap_handler_context
{
    // yield是每个章节自己的，不同的调度策略会调用不同的sched，而ch2里面甚至不需要支持（空函数）
	void (*yield)();

    // set_usertrap和set_kerneltrap分别用于处理在U/S态下发生异常时的异常处理代码入口
	void (*set_usertrap)();
	void (*set_kerneltrap)();

    // 用于得到当前正在考虑的trapframe,在ch2里面是全局共用的，到后面就是不同进程、再到不同线程的trap_frame
	struct trapframe* (*get_trapframe)();
    // 用于拿到内核栈的栈顶指针
	uint64 (*get_kernel_sp)();

    // 放在usertrapret的最后执行，是在从S态回到U态前要做的一些工作，如把页表地址取出，然后调用userret最终回到S态
	void (*call_userret)();
    // 在usertrap执行结束后执行，cause是异常编号，这里主要是为了兼容ch2，后面的章节里面只要调用usertrapret即可
	void (*finish_usertrap)(int cause);
    // 用于处理当trap处理发生错误时要做的事
	void (*error_in_trap)(int status);
	// 用于处理非时间中断的一切外部中断，会在文件系统里面用到
	void (*super_external_handler)();
};
```
# kernel-vm
## kalloc
> 内存分配

```c
// 内存分配模块，采用指针列表实现
extern char ekernel[];

struct linklist {
	struct linklist* next;
};

struct {
	struct linklist* freelist;
} kmem;

void* kalloc();
void kfree(void*);
void kinit();
```
## vm
> 工具函数

```c
// 两种功能：地址转换；建立映射
pte_t* walk(pagetable_t, uint64, int);
// 虚拟地址转物理地址函数
uint64 walkaddr(pagetable_t, uint64);
uint64 useraddr(pagetable_t, uint64);
// 释放地址空间函数
void freewalk(pagetable_t);
// 用户态和内核态拷贝函数
int copyout(pagetable_t, uint64, char*, uint64);
int copyin(pagetable_t, char*, uint64, uint64);
int copyinstr(pagetable_t, char*, uint64, uint64);
```
## map
> 映射函数

```c
extern char trampoline[];

// 为内核分配虚拟地址空间
void kvmmap(pagetable_t, uint64, uint64, uint64, int);
// 建立虚拟地址到物理地址映射
int mappages(pagetable_t, uint64, uint64, uint64, int);
int uvmmap(pagetable_t, uint64, uint64, int);
// 释放虚拟地址到物理地址映射
void uvmunmap(pagetable_t, uint64, uint64, int);
// 创建空用户页表
pagetable_t uvmcreate(uint64);
// 释放用户地址空间
void uvmfree(pagetable_t, uint64);
// 拷贝用户地址空间
int uvmcopy(pagetable_t, pagetable_t, uint64);
```
# task-manage
## manager
> 任务的操作接口

```c
/*
注意返回类型为void*的接口，
实际返回类型就是proc结构体指针，
但proc在不同的lab中定义不同，
因此采用无类型指针定义
*/
struct manager {
    // 创建任务
    void (*create)();
    // 删除任务
    void (*delete)(void* p);
    // 根据id获取任务
    void* (*get)(int id);
    // 将任务加入调度队列
    void (*add)(void* p);
    // 从调度队列中取出相应任务
    void* (*fetch)(); 
};
```
## processor
> 负责任务的调度

```c
#include "manager.h"

void* current_proc;
struct manager* proc_manager;

// manager各个接口的实现在各个lab中，在这里初始化proc_manager
void set_manager(struct manager* lab_manager) {
	proc_manager = lab_manager;
}
// 获得当前正在运行的进程
void* curr_proc() {
	return current_proc;
}
// 修改当前正在运行的进程
void set_curr(void* p) {
	current_proc = p;
}
// 根据id获得进程
void* get_proc(int id) {
	return proc_manager->get(id);
}
// 创建进程
void* alloc_proc() {
	return proc_manager->create();
}
// 释放进程
void free_proc(void* p) {
	proc_manager->delete(p);
}
// 阻塞进程
void add_task(void* p) {
	proc_manager->add(p);
}
// 调度进程
void* fetch_task() {
	return proc_manager->fetch();
}
```
## queue
> 队列类

```c
// 进程调度模块，采用循环队列实现
struct queue {
	int data[QUEUE_SIZE];
	int front;
	int tail;
	int empty;
};

void init_queue(struct queue*);
void push_queue(struct queue*, int);
int pop_queue(struct queue*);
```
## lab
> 每个章节实现不同

```c
struct proc {
    // lab5
	enum procstate state; // 运行状态
	int pid; // 进程号
	uint64 max_page; // 最大页地址
	struct proc* parent; // 父进程
	uint64 exit_code; // 退出码
    
    // 以下需配合其它模块设计
    uint64 ustack; // 用户栈
	uint64 kstack; // 内核栈
    pagetable_t pagetable; // 页表
	struct trapframe* trapframe; // 陷入上下文
	struct context context; // 进程上下文

    // lab6
	struct file* files[FD_BUFFER_SIZE];
};
```
```c
#include "processor.h"
#include "proc.h"
#include "queue.h"

struct proc pool[NPROC];
struct proc idle;
struct queue task_queue; // queue的实现在单独的工具类里

// 5个接口的实现
void create(){}
void delete(struct proc* p) {}
struct proc* get(int id) {}
void add(struct proc* p) {}
struct proc* fetch() {}

struct manager m = {
    .create = create,
    .delete = delete,
    .get = get,
    .add = add,
    .fetch = fetch
};

void init_manager() {
	set_manager(&m);
    // 其它初始化工作
}

/* 
以下函数的实现调用processor.c中的函数,
同时也要结合其它模块的函数(如kernel-vm),
应达到实现syscall中定义的相关接口的目标
*/
int fork() {}
int exec(char* name) {}
int wait(int pid, int *code) {}
void exit(int code) {}
void sched() {}
void yield() {}
```
# easy-fs
## 结构体
```c
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

// 该结构体提供有关文件当前使用情况和相应inode位置的信息
struct file {
	enum { FD_NONE = 0, FD_INODE, FD_STDIO } type;
	int ref; // reference count
	char readable;
	char writable;
	struct inode *ip; // FD_INODE
	uint off;
};

struct FSManager{
    void (*fileclose)(struct file *);
    int (*fileopen)(char *, uint64);
    int (*link)(char *src, char *dst);
    int (*unlink)(char *path);
    int (*readdir)(char *path);
}
```
```c
struct superblock {
	uint magic; // Must be FSMAGIC
	uint size; // Size of file system image (blocks)
	uint nblocks; // Number of data blocks
	uint ninodes; // Number of inodes.
	uint inodestart; // Block number of first inode block
	uint bmapstart; // Block number of first free map block
};

// On-disk inode structure
struct dinode {
	short type; // File type
	short pad[3];
	// LAB4: you can reduce size of pad array and add link count below,
	//       or you can just regard a pad as link count.
	//       But keep in mind that you'd better keep sizeof(dinode) unchanged
	uint size; // Size of file (bytes)
	uint addrs[NDIRECT + 1]; // Data block addresses
};

struct dirent {
	ushort inum;
	char name[DIRSIZ];
};
```
## 函数
```c
struct file *filealloc();
uint64 inodewrite(struct file *, uint64, uint64);
uint64 inoderead(struct file *, uint64, uint64);
struct file *stdio_init(int);
int show_all_files();
```
```c
void fsinit();
int dirlink(struct inode *, char *, uint);
struct inode *dirlookup(struct inode *, char *, uint *);
struct inode *ialloc(uint, short);
struct inode *idup(struct inode *);
void iinit();
void ivalid(struct inode *);
void iput(struct inode *);
void iunlock(struct inode *);
void iunlockput(struct inode *);
void iupdate(struct inode *);
struct inode *namei(char *);
struct inode *root_dir();
int readi(struct inode *, int, uint64, uint, uint);
int writei(struct inode *, int, uint64, uint, uint);
void itrunc(struct inode *);
int dirls(struct inode *);
```
```c
struct FSManager fsmanger = {
    .fileclose = fileclose;
    .fileopen = fileopen;
    .link = link;
    .unlink = unlink;
    .readdir = readdir;
}

void fileclose(struct file *);
int fileopen(char *, uint64);
int link(char *src, char *dst);
int unlink(char *path);
int readdir(char *path);
```
