#ifndef STRING_H
#define STRING_H

/**
 * @file string.h
 * @brief 实现了简单的字符串和内存操作函数，很多模块可以视情况使用
*/

#include "defs.h"

int memcmp(const void *, const void *, uint); ///< 比较内存数据
void *memmove(void *, const void *, uint); ///< 拷贝内存数据
void *memset(void *, int, uint); ///< 设置内存数据
char *safestrcpy(char *, const char *, int); ///< 拷贝字符串
int strlen(const char *); ///< 字符串长度
int strncmp(const char *, const char *, uint); ///< 字符串比较
char *strncpy(char *, const char *, int); ///< 字符串拷贝

#endif // STRING_H