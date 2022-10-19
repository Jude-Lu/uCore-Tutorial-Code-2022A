#ifndef LOADER_H
#define LOADER_H

#include "../utils/defs.h"
#include "../utils/modules.h"
#include "proc.h"

int finished();
void loader_init();
int load_init_app();
int loader(int, struct proc *);
int get_id_by_name(char *);

#define BASE_ADDRESS (0x1000)
#define USTACK_SIZE (PAGE_SIZE)
#define KSTACK_SIZE (PAGE_SIZE)
#define TRAP_PAGE_SIZE (PAGE_SIZE)

#endif // LOADER_H