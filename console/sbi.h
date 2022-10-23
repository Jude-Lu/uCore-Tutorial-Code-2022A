#ifndef SBI_H
#define SBI_H

#include "defs.h"

void console_putchar(int);
int console_getchar();
void shutdown();
void set_timer(uint64);

#endif // SBI_H
