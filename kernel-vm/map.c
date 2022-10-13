#include "map.h"
#include "vm.h"
#include "kalloc.h"
#include "../utils/defs.h"

// Add a mapping to the kernel page table.
// only used when booting.
// does not flush TLB or enable paging.
void kvmmap(pagetable_t kpgtbl, uint64 va, uint64 pa, uint64 sz, int perm) {
    if (mappages(kpgtbl, va, sz, pa, perm) != 0)
        panic("kvmmap");
}

// Create PTEs for virtual addresses starting at va that refer to
// physical addresses starting at pa. va and size might not
// be page-aligned. Returns 0 on success, -1 if walk() couldn't
// allocate a needed page-table page.
int mappages(pagetable_t pagetable, uint64 va, uint64 size, uint64 pa, int perm) {
    uint64 a, last;
    pte_t* pte;

    a = PGROUNDDOWN(va);
    last = PGROUNDDOWN(va + size - 1);
    for (;;) {
        if ((pte = walk(pagetable, a, 1)) == 0) {
            errorf("pte invalid, va = %p", a);
            return -1;
        }
        if (*pte & PTE_V) {
            errorf("remap");
            return -1;
        }
        *pte = PA2PTE(pa) | perm | PTE_V;
        if (a == last)
            break;
        a += PGSIZE;
        pa += PGSIZE;
    }
    return 0;
}

int uvmmap(pagetable_t pagetable, uint64 va, uint64 npages, int perm) {
    for (int i = 0; i < npages; ++i) {
        if (mappages(pagetable, va + i * 0x1000, 0x1000,
                     (uint64)kalloc(), perm)) {
            return -1;
        }
    }
    return 0;
}

// Remove npages of mappings starting from va. va must be
// page-aligned. The mappings must exist.
// Optionally free the physical memory.
void uvmunmap(pagetable_t pagetable, uint64 va, uint64 npages, int do_free) {
    uint64 a;
    pte_t* pte;

    if ((va % PGSIZE) != 0)
        panic("uvmunmap: not aligned");

    for (a = va; a < va + npages * PGSIZE; a += PGSIZE) {
        if ((pte = walk(pagetable, a, 0)) == 0)
            continue;
        if ((*pte & PTE_V) != 0) {
            if (PTE_FLAGS(*pte) == PTE_V)
                panic("uvmunmap: not a leaf");
            if (do_free) {
                uint64 pa = PTE2PA(*pte);
                kfree((void*)pa);
            }
        }
        *pte = 0;
    }
}

// create an empty user page table.
// returns 0 if out of memory.
pagetable_t uvmcreate() {
    pagetable_t pagetable;
    pagetable = (pagetable_t)kalloc();
    if (pagetable == 0) {
        errorf("uvmcreate: kalloc error");
        return 0;
    }
    memset(pagetable, 0, PGSIZE);
    if (mappages(pagetable, TRAMPOLINE, PAGE_SIZE, (uint64)trampoline,
                 PTE_R | PTE_X) < 0) {
        panic("mappages fail");
    }
    return pagetable;
}

/**
 * @brief Free user memory pages, then free page-table pages.
 *
 * @param max_page The max vaddr of user-space.
 */
void uvmfree(pagetable_t pagetable, uint64 max_page) {
    if (max_page > 0)
        uvmunmap(pagetable, 0, max_page, 1);
    freewalk(pagetable);
}

// Used in fork.
// Copy the pagetable page and all the user pages.
// Return 0 on success, -1 on error.
int uvmcopy(pagetable_t old, pagetable_t new, uint64 max_page) {
    pte_t* pte;
    uint64 pa, i;
    uint flags;
    char* mem;

    for (i = 0; i < max_page * PAGE_SIZE; i += PGSIZE) {
        if ((pte = walk(old, i, 0)) == 0)
            continue;
        if ((*pte & PTE_V) == 0)
            continue;
        pa = PTE2PA(*pte);
        flags = PTE_FLAGS(*pte);
        if ((mem = kalloc()) == 0)
            goto err;
        memmove(mem, (char*)pa, PGSIZE);
        if (mappages(new, i, PGSIZE, (uint64)mem, flags) != 0) {
            kfree(mem);
            goto err;
        }
    }
    return 0;

err:
    uvmunmap(new, 0, i / PGSIZE, 1);
    return -1;
}
