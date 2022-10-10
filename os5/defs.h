#ifndef DEFS_H
#define DEFS_H

#include "../utils/const.h"
#include "kalloc.h"
#include "../utils/log.h"
#include "../utils/printf.h"
#include "proc.h"
#include "../utils/riscv.h"
#include "../utils/sbi.h"
#include "../utils/string.h"
#include "../utils/types.h"
#include "vm.h"

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#define NULL ((void *)0)

#endif // DEF_H
