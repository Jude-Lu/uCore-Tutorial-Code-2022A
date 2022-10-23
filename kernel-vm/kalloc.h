#ifndef KALLOC_H
#define KALLOC_H

#include "defs.h"
#include "log.h"

void* kalloc();
void kfree(void*);
void kinit();

#endif  // KALLOC_H