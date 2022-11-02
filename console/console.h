#ifndef CONSOLE_H
#define CONSOLE_H

/**
 * @file console.h
 * @brief 控制台文件，调用sbi实现字符的输入输出
*/

void consputc(int); ///< 控制台输出字符
int consgetc(); ///< 控制台输入字符
void console_init(); ///< 控制台初始化

#endif // CONSOLE_H
