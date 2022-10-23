#ifndef KALLOC_H
#define KALLOC_H

#include "defs.h"
#include "log.h"

extern void *memmove(void *, const void *, uint);
extern void *memset(void *, int, uint);

void* kalloc();
void kfree(void*);
void kinit();

#endif  // KALLOC_H