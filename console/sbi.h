#ifndef SBI_H
#define SBI_H

typedef unsigned int uint;
typedef unsigned short ushort;
typedef unsigned char uchar;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned int uint32;
typedef unsigned long uint64;

void console_putchar(int);
int console_getchar();
void shutdown();
void set_timer(uint64);

#endif // SBI_H
