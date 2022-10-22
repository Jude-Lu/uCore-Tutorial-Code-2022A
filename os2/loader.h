#ifndef LOADER_H
#define LOADER_H

void loader_init();
int run_next_app();

#define PAGE_SIZE (0x1000)
#define BASE_ADDRESS (0x80400000)
#define MAX_APP_SIZE (0x20000)
#define USER_STACK_SIZE PAGE_SIZE
#define TRAP_PAGE_SIZE PAGE_SIZE

#endif // LOADER_H