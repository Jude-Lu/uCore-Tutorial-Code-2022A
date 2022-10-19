#include "vm.h"
#include "kalloc.h"
#include "../utils/defs.h"

/**
 * Return the address of the PTE in page table pagetable
 * that corresponds to virtual address va.  If alloc!=0,
 * create any required page-table pages.
 * The risc-v Sv39 scheme has three levels of page-table
 * pages. A page-table page contains 512 64-bit PTEs.
 * A 64-bit virtual address is split into five fields:
 * 39..63 -- must be zero.
 * 30..38 -- 9 bits of level-2 index.
 * 21..29 -- 9 bits of level-1 index.
 * 12..20 -- 9 bits of level-0 index.
 * 0..11 -- 12 bits of byte offset within the page.
 */  
pte_t* walk(pagetable_t pagetable, uint64 va, int alloc) {
    if (va >= MAXVA)
        panic("walk");

    for (int level = 2; level > 0; level--) {
        pte_t* pte = &pagetable[PX(level, va)];
        if (*pte & PTE_V) {
            pagetable = (pagetable_t)PTE2PA(*pte);
        } else {
            if (!alloc || (pagetable = (pde_t*)kalloc()) == 0)
                return 0;
            memset(pagetable, 0, PGSIZE);
            *pte = PA2PTE(pagetable) | PTE_V;
        }
    }
    return &pagetable[PX(0, va)];
}

/**
 * Look up a virtual address, return the physical address,
 * or 0 if not mapped.
 * Can only be used to look up user pages.
 */
uint64 walkaddr(pagetable_t pagetable, uint64 va) {
    pte_t* pte;
    uint64 pa;

    if (va >= MAXVA)
        return 0;

    pte = walk(pagetable, va, 0);
    if (pte == 0)
        return 0;
    if ((*pte & PTE_V) == 0)
        return 0;
    if ((*pte & PTE_U) == 0)
        return 0;
    pa = PTE2PA(*pte);
    return pa;
}

/// Look up a virtual address, return the physical address,
uint64 useraddr(pagetable_t pagetable, uint64 va) {
    uint64 page = walkaddr(pagetable, va);
    if (page == 0)
        return 0;
    return page | (va & 0xFFFULL);
}

/**
 * Recursively free page-table pages.
 * All leaf mappings must already have been removed.
 */
void freewalk(pagetable_t pagetable) {
    // there are 2^9 = 512 PTEs in a page table.
    for (int i = 0; i < 512; i++) {
        pte_t pte = pagetable[i];
        if ((pte & PTE_V) && (pte & (PTE_R | PTE_W | PTE_X)) == 0) {
            // this PTE points to a lower-level page table.
            uint64 child = PTE2PA(pte);
            freewalk((pagetable_t)child);
            pagetable[i] = 0;
        } else if (pte & PTE_V) {
            panic("freewalk: leaf");
        }
    }
    kfree((void*)pagetable);
}

/**
 * Copy from kernel to user.
 * Copy len bytes from src to virtual address dstva in a given page table.
 * Return 0 on success, -1 on error.
 */
int copyout(pagetable_t pagetable, uint64 dstva, char* src, uint64 len) {
    uint64 n, va0, pa0;

    while (len > 0) {
        va0 = PGROUNDDOWN(dstva);
        pa0 = walkaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;
        n = PGSIZE - (dstva - va0);
        if (n > len)
            n = len;
        memmove((void*)(pa0 + (dstva - va0)), src, n);

        len -= n;
        src += n;
        dstva = va0 + PGSIZE;
    }
    return 0;
}

/**
 * Copy from user to kernel.
 * Copy len bytes to dst from virtual address srcva in a given page table.
 * Return 0 on success, -1 on error.
 */
int copyin(pagetable_t pagetable, char* dst, uint64 srcva, uint64 len) {
    uint64 n, va0, pa0;

    while (len > 0) {
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;
        n = PGSIZE - (srcva - va0);
        if (n > len)
            n = len;
        memmove(dst, (void*)(pa0 + (srcva - va0)), n);

        len -= n;
        dst += n;
        srcva = va0 + PGSIZE;
    }
    return 0;
}

/**
 * Copy a null-terminated string from user to kernel.
 * Copy bytes to dst from virtual address srcva in a given page table,
 * until a '\0', or max.
 * Return 0 on success, -1 on error.
 */
int copyinstr(pagetable_t pagetable, char* dst, uint64 srcva, uint64 max) {
    uint64 n, va0, pa0;
    int got_null = 0, len = 0;

    while (got_null == 0 && max > 0) {
        va0 = PGROUNDDOWN(srcva);
        pa0 = walkaddr(pagetable, va0);
        if (pa0 == 0)
            return -1;
        n = PGSIZE - (srcva - va0);
        if (n > max)
            n = max;

        char* p = (char*)(pa0 + (srcva - va0));
        while (n > 0) {
            if (*p == '\0') {
                *dst = '\0';
                got_null = 1;
                break;
            } else {
                *dst = *p;
            }
            --n;
            --max;
            p++;
            dst++;
            len++;
        }

        srcva = va0 + PGSIZE;
    }
    return len;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int either_copyout(pagetable_t pagetable, int user_dst, uint64 dst, char *src, uint64 len)
{
	if (user_dst) {
		return copyout(pagetable, dst, src, len);
	} else {
		memmove((void *)dst, src, len);
		return 0;
	}
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int either_copyin(pagetable_t pagetable, int user_src, uint64 src, char *dst, uint64 len)
{
	if (user_src) {
		return copyin(pagetable, dst, src, len);
	} else {
		memmove(dst, (char *)src, len);
		return 0;
	}
}