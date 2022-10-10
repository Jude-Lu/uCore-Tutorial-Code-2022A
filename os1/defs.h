#ifndef DEFS_H
#define DEFS_H

#include "../utils/log.h"
#include "../utils/printf.h"
#include "../utils/riscv.h"
#include "../utils/sbi.h"
#include "../utils/types.h"

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))

#endif // DEF_H
