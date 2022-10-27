.PHONY: clean build .FORCE
all: build

MOD = pipe
DIR = $(MOD)
ACHV = lib$(MOD).a

TOOLPREFIX = riscv64-unknown-elf-
CC = $(TOOLPREFIX)gcc
AR = $(TOOLPREFIX)ar
BUILDDIR = ../build

C_SRCS = $(wildcard *.c)
C_OBJS = $(addsuffix .o, $(basename $(C_SRCS)))
OBJS = $(C_OBJS)
HEADER_DEP = $(addsuffix .d, $(basename $(C_OBJS)))

-include $(HEADER_DEP)

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += $(shell $(CC) -fno-stack-protector -E -x c /dev/null >/dev/null 2>&1 && echo -fno-stack-protector)

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
	$(CC) $(CFLAGS) -c $< -o $@

$(HEADER_DEP): %.d : %.c
	@mkdir -p $(BUILDDIR)/$(MOD)
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
        sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$
	@mv $@ $(BUILDDIR)/$(MOD)

build: $(OBJS)
	$(AR) -rc $(ACHV) $(OBJS)
	@mv $(OBJS) *.d -t $(BUILDDIR)/$(MOD)
	@mkdir -p $(BUILDDIR)/lib
	@mv $(ACHV) $(BUILDDIR)/lib

clean:
	@rm -rf *.o *.a *.d

