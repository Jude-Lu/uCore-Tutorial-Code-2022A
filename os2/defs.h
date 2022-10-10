#ifndef DEFS_H
#define DEFS_H

#include "../utils/const.h"
#include "../utils/log.h"
#include "../utils/printf.h"
#include "../utils/riscv.h"
#include "../utils/sbi.h"
#include "../utils/string.h"
#include "../utils/types.h"

// number of elements in fixed-size array
#define NELEM(x) (sizeof(x) / sizeof((x)[0]))
#define MIN(a, b) (a < b ? a : b)
#define MAX(a, b) (a > b ? a : b)

#endif // DEF_H
