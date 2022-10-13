#ifndef DEFS_H
#define DEFS_H

#include "const.h"
#include "log.h"
#include "printf.h"
#include "riscv.h"
#include "sbi.h"
#include "string.h"
#include "types.h"
#include "timer.h"
#include "console.h"
#include "../syscall/syscall.h"
#include "../trap/trap.h"
#include "../kernel-vm/kalloc.h"
#include "../kernel-vm/map.h"
#include "../kernel-vm/vm.h"

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define NULL ((void *)0)

#endif // DEF_H
