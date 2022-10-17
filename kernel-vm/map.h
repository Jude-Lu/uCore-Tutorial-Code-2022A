#ifndef MAP_H
#define MAP_H

#include "../utils/riscv.h"
#include "../utils/types.h"

/// 跳板页地址
extern char trampoline[];

/// 为内核分配虚拟地址空间
void kvmmap(pagetable_t, uint64, uint64, uint64, int);
/// 建立虚拟地址到物理地址映射
int mappages(pagetable_t, uint64, uint64, uint64, int);
int uvmmap(pagetable_t, uint64, uint64, int);
/// 释放虚拟地址到物理地址映射
void uvmunmap(pagetable_t, uint64, uint64, int);
/// 创建空用户页表
pagetable_t uvmcreate();
/// 释放用户地址空间
void uvmfree(pagetable_t, uint64);
/// 拷贝用户地址空间
int uvmcopy(pagetable_t, pagetable_t, uint64);

#endif  // MAP_H