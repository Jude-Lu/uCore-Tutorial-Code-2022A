.PHONY: clean build user run debug test .FORCE
all: build

ch ?= 1
MOD = syscall
DIR = $(MOD)
ACHV = lib$(MOD).a

TOOLPREFIX = riscv64-unknown-elf-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gcc
AR = $(TOOLPREFIX)ar
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
PY = python3
GDB = $(TOOLPREFIX)gdb
CP = cp
BUILDDIR = ../build

## Append your module dir
C_SRCS = $(wildcard *.c)
$(info $(C_SRCS))
##

C_OBJS = $(addsuffix .o, $(basename $(C_SRCS)))
OBJS = $(C_OBJS)
$(info $(C_OBJS))
HEADER_DEP = $(addsuffix .d, $(basename $(C_OBJS)))
$(info $(HEADER_DEP))

-include $(HEADER_DEP)

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
# CFLAGS += -I$K
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

LOG ?= error

ifeq ($(LOG), error)
CFLAGS += -D LOG_LEVEL_ERROR
else ifeq ($(LOG), warn)
CFLAGS += -D LOG_LEVEL_WARN
else ifeq ($(LOG), info)
CFLAGS += -D LOG_LEVEL_INFO
else ifeq ($(LOG), debug)
CFLAGS += -D LOG_LEVEL_DEBUG
else ifeq ($(LOG), trace)
CFLAGS += -D LOG_LEVEL_TRACE
endif

# Disable PIE when possible (for Ubuntu 16.10 toolchain)
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]no-pie'),)
CFLAGS += -fno-pie -no-pie
endif
ifneq ($(shell $(CC) -dumpspecs 2>/dev/null | grep -e '[^f]nopie'),)
CFLAGS += -fno-pie -nopie
endif

# empty target
.FORCE:

$(C_OBJS): %.o : %.c %.d
	@mkdir -p $(BUILDDIR)/$(MOD)
	@echo $(BUILDDIR)/$(MOD)
	$(CC) $(CFLAGS) -c $< -o $@
	# @cp $@ $(BUILDDIR)/$(MOD)

$(HEADER_DEP): %.d : %.c
	@mkdir -p $(BUILDDIR)/$(MOD)
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
        sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$
	@mv $@ $(BUILDDIR)/$(MOD)

build: $(OBJS)
	$(AR) -rc $(ACHV) $(OBJS)
	@mv $(OBJS) -t $(BUILDDIR)/$(MOD)
	@mkdir -p $(BUILDDIR)/lib
	@mv $(ACHV) $(BUILDDIR)/lib

clean:
	rm -rf *.o *.a *.d


