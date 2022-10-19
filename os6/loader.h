#ifndef LOADER_H
#define LOADER_H

#include "../utils/defs.h"
#include "../utils/modules.h"
#include "../easy-fs/file.h"
#include "proc.h"

int load_init_app();
int bin_loader(struct inode *, struct proc *);

#define BASE_ADDRESS (0x1000)
#define USTACK_SIZE (PAGE_SIZE)
#define KSTACK_SIZE (PAGE_SIZE)
#define TRAP_PAGE_SIZE (PAGE_SIZE)

#endif // LOADER_H