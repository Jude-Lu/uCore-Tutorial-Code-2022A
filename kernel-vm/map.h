#ifndef MAP_H
#define MAP_H

#include "../utils/riscv.h"
#include "../utils/types.h"

extern char trampoline[];

void kvmmap(pagetable_t, uint64, uint64, uint64, int);
int mappages(pagetable_t, uint64, uint64, uint64, int);
int uvmmap(pagetable_t, uint64, uint64, int);
void uvmunmap(pagetable_t, uint64, uint64, int);
pagetable_t uvmcreate();
void uvmfree(pagetable_t, uint64);
int uvmcopy(pagetable_t, pagetable_t, uint64);

#endif  // MAP_H