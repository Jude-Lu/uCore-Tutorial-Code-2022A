.PHONY: clean build user run debug test .FORCE
all: build

ch ?= 1
K = os$(ch)
U = user

TOOLPREFIX = riscv64-unknown-elf-
CC = $(TOOLPREFIX)gcc
AS = $(TOOLPREFIX)gcc
LD = $(TOOLPREFIX)ld
OBJCOPY = $(TOOLPREFIX)objcopy
OBJDUMP = $(TOOLPREFIX)objdump
PY = python3
GDB = $(TOOLPREFIX)gdb
CP = cp
BUILDDIR = build
C_SRCS = $(wildcard $K/*.c)
AS_SRCS = $(wildcard $K/*.S)
C_OBJS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(C_SRCS))))
AS_OBJS = $(addprefix $(BUILDDIR)/, $(addsuffix .o, $(basename $(AS_SRCS))))
OBJS = $(C_OBJS) $(AS_OBJS)

HEADER_DEP = $(addsuffix .d, $(basename $(C_OBJS)))

ifeq ($(shell expr $(ch) \<= 5)$(shell expr $(ch) \>= 2), 11)
ifeq (,$(findstring link_app.o,$(OBJS)))
	AS_OBJS += $(BUILDDIR)/$K/link_app.o
endif
else ifeq ($(shell expr $(ch) \>= 6), 1)
F = $K/nfs
ifeq (,$(findstring initproc.o,$(OBJS)))
	AS_OBJS += $(BUILDDIR)/$K/initproc.o
endif
endif

CFLAGS = -Wall -Werror -O -fno-omit-frame-pointer -ggdb
CFLAGS += -MD
CFLAGS += -mcmodel=medany
CFLAGS += -ffreestanding -fno-common -nostdlib -mno-relax
CFLAGS += -I$K
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

LDFLAGS = -z max-page-size=4096

$(AS_OBJS): $(BUILDDIR)/$K/%.o : $K/%.S
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(C_OBJS): $(BUILDDIR)/$K/%.o : $K/%.c  $(BUILDDIR)/$K/%.d
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) -c $< -o $@

$(HEADER_DEP): $(BUILDDIR)/$K/%.d : $K/%.c
	@mkdir -p $(@D)
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
        sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$

ifeq ($(shell expr $(ch) \>= 6), 1)
INIT_PROC ?= usershell
$(K)/initproc.o: $K/initproc.S
$(K)/initproc.S: $K/initproc.py .FORCE
	cd $K && $(PY) initproc.py $(INIT_PROC)

else ifeq ($(shell expr $(ch) \== 5), 1)
INIT_PROC ?= usershell
$(K)/link_app.o: $K/link_app.S
$(K)/link_app.S: $K/pack.py .FORCE
	cd $K && $(PY) pack.py $(INIT_PROC)
$(K)/kernel_app.ld: $K/kernelld.py .FORCE
	cd $K && $(PY) kernelld.py

else ifeq ($(shell expr $(ch) \== 1), 1)

else
os$(ch)/link_app.o: $K/link_app.S
os$(ch)/link_app.S: $K/pack.py
	cd $K && $(PY) pack.py
os$(ch)/kernel_app.ld: $K/kernelld.py
	cd $K && $(PY) kernelld.py
endif


INIT_PROC ?= usershell

build: build/kernel

build/kernel: $(OBJS) os$(ch)/kernel.ld
	$(LD) $(LDFLAGS) -T os$(ch)/kernel.ld -o $(BUILDDIR)/kernel $(OBJS)
	$(OBJDUMP) -S $(BUILDDIR)/kernel > $(BUILDDIR)/kernel.asm
	$(OBJDUMP) -t $(BUILDDIR)/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(BUILDDIR)/kernel.sym
	@echo 'Build kernel done'

clean:
	rm -rf $(BUILDDIR)

# BOARD
BOARD		?= qemu
SBI			?= rustsbi
BOOTLOADER	:= ./bootloader/rustsbi-qemu.bin

QEMU = qemu-system-riscv64

# QEMU's gdb stub command line changed in 0.11
QEMUGDB = $(shell if $(QEMU) -help | grep -q '^-gdb'; \
	then echo "-gdb tcp::15234"; \
	else echo "-s -p 15234"; fi)

ifeq ($(shell expr $(ch) \>= 6), 1)
QEMUOPTS = \
	-nographic \
	-machine virt \
	-bios $(BOOTLOADER) \
	-kernel build/kernel	\
	-drive file=$(F)/fs-copy.img,if=none,format=raw,id=x0 \
    -device virtio-blk-device,drive=x0,bus=virtio-mmio-bus.0

$(F)/fs.img:
	make -C $(F)

$(F)/fs-copy.img: $(F)/fs.img
	@$(CP) $< $@

run: build/kernel $(F)/fs-copy.img
	$(QEMU) $(QEMUOPTS)

debug: build/kernel .gdbinit $(F)/fs-copy.img
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB) &
	sleep 1
	$(GDB)
else
QEMUOPTS = \
	-nographic \
	-machine virt \
	-bios $(BOOTLOADER) \
	-kernel build/kernel    \

run: build/kernel
	$(QEMU) $(QEMUOPTS)

debug: build/kernel .gdbinit
	$(QEMU) $(QEMUOPTS) -S $(QEMUGDB) &
	sleep 1
	$(GDB)
endif

user:
	make -C user CHAPTER=$(ch) BASE=$(BASE)

test: user run

