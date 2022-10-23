#ifndef VM_H
#define VM_H

#include "defs.h"
#include "log.h"

/// 两种功能：地址转换；建立映射
pte_t* walk(pagetable_t, uint64, int);
/// 虚拟地址转物理地址函数
uint64 walkaddr(pagetable_t, uint64);
uint64 useraddr(pagetable_t, uint64);
/// 释放地址空间函数
void freewalk(pagetable_t);
/// 用户态和内核态拷贝函数
int copyout(pagetable_t, uint64, char*, uint64);
int copyin(pagetable_t, char*, uint64, uint64);
int copyinstr(pagetable_t, char*, uint64, uint64);

int either_copyout(pagetable_t, int, uint64, char*, uint64);
int either_copyin(pagetable_t, int, uint64, char*, uint64);

#endif  // VM_H