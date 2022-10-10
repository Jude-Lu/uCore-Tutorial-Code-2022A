#ifndef VM_H
#define VM_H

#include "../utils/riscv.h"
#include "../utils/types.h"

pte_t* walk(pagetable_t, uint64, int);
uint64 walkaddr(pagetable_t, uint64);
uint64 useraddr(pagetable_t, uint64);
void freewalk(pagetable_t);
int copyout(pagetable_t, uint64, char*, uint64);
int copyin(pagetable_t, char*, uint64, uint64);
int copyinstr(pagetable_t, char*, uint64, uint64);

#endif  // VM_H