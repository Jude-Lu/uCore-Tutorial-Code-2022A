#include "plic.h"
#include "defs.h"

//
// the riscv Platform Level Interrupt Controller (PLIC).
//

void plicinit(int hart)
{
	// set desired IRQ priorities non-zero (otherwise disabled).
	*(uint32 *)(PLIC + VIRTIO0_IRQ * 4) = 1;
	// set uart's enable bit for this hart's S-mode.
	*(uint32 *)PLIC_SENABLE(hart) = (1 << VIRTIO0_IRQ);
	// set this hart's S-mode priority threshold to 0.
	*(uint32 *)PLIC_SPRIORITY(hart) = 0;
}

// ask the PLIC what interrupt we should serve.
int plic_claim(int hart)
{
	int irq = *(uint32 *)PLIC_SCLAIM(hart);
	return irq;
}

// tell the PLIC we've served this IRQ.
void plic_complete(int hart, int irq)
{
	*(uint32 *)PLIC_SCLAIM(hart) = irq;
}
