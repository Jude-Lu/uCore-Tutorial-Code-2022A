#include "loader.h"
#include "proc.h"
#include "os4_syscall.h"
#include "os4_trap.h"

void clean_bss()
{
	memset(s_bss, 0, e_bss - s_bss);
}

// Make a direct-map page table for the kernel.
pagetable_t kvmmake(void)
{
	pagetable_t kpgtbl;
	kpgtbl = (pagetable_t)kalloc();
	memset(kpgtbl, 0, PGSIZE);
	// map kernel text executable and read-only.
	kvmmap(kpgtbl, KERNBASE, KERNBASE, (uint64)e_text - KERNBASE,
	       PTE_R | PTE_X);
	// map kernel data and the physical RAM we'll make use of.
	kvmmap(kpgtbl, (uint64)e_text, (uint64)e_text, PHYSTOP - (uint64)e_text,
	       PTE_R | PTE_W);
	kvmmap(kpgtbl, TRAMPOLINE, (uint64)trampoline, PGSIZE, PTE_R | PTE_X);
	return kpgtbl;
}

// Initialize the one kernel_pagetable
// Switch h/w page table register to the kernel's page table,
// and enable paging.
void kvm_init(void)
{
	pagetable_t kernel_pagetable = kvmmake();
	w_satp(MAKE_SATP(kernel_pagetable));
	sfence_vma();
	infof("enable pageing at %p", r_satp());
}

void main()
{
	clean_bss();
	printf("hello world!\n");
	proc_init();
	kinit();
	kvm_init();
	loader_init();
	trap_init();
	timer_init();
	syscall_init();
	run_all_app();
	infof("start scheduler!");
	scheduler();
}