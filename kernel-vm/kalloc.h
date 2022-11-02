#ifndef KALLOC_H
#define KALLOC_H

/**
 * @file kalloc.h
 * @brief 主要负责物理内存的管理
 */

#include "defs.h"
#include "log.h"

void* kalloc(); ///< 分配一块内存
void kfree(void*); ///< 释放一块内存
void kinit(); ///< 内存分配初始化

#endif  // KALLOC_H