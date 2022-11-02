#ifndef SBI_H
#define SBI_H

/**
 * @file sbi.h
 * @brief sbi文件，通过ecall实现最底层的输入输出
*/

#include "defs.h"

void console_putchar(int); ///< 调用ecall实现putchar
int console_getchar(); ///< 调用ecall实现getchar
void shutdown(); ///< 调用ecall实现关机
void set_timer(uint64); ///< 调用ecall设置下一时钟中断

#endif // SBI_H
