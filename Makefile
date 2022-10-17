.PHONY: clean build user run debug test .FORCE
all: build

ch ?= 1
K = os$(ch)
U = user
F = nfs

## Add your module dir here
ASM = asm
UTIL = utils
SYSCALL = syscall
TRAP = trap
VM = kernel-vm
TASK = task-manage
SCRIPT = script
##

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

## Append your module dir
C_SRCS = $(wildcard $(K)/*.c $(UTIL)/*.c)
ifeq ($(shell expr $(ch) \>= 2), 1)
	C_SRCS += $(wildcard $(SYSCALL)/*.c)
endif
ifeq ($(shell expr $(ch) \>= 2), 1)
	C_SRCS += $(wildcard $(TRAP)/*.c)
endif
ifeq ($(shell expr $(ch) \>= 3), 1)
	C_SRCS += $(wildcard $(TASK)/*.c)
endif
ifeq ($(shell expr $(ch) \>= 4), 1)
	C_SRCS += $(wildcard $(VM)/*.c)
endif
##

AS_SRCS = $(wildcard $K/*.S $(ASM)/entry.S)
ifeq ($(shell expr $(ch) \>= 2), 1)
	AS_SRCS += $(TRAP)/trampoline.S
endif
ifeq ($(shell expr $(ch) \>= 3), 1)
	AS_SRCS += $(TRAP)/switch.S
endif
ifeq ($(shell expr $(ch) \>= 5), 1)
	AS_SRCS += $(ASM)/initproc.S
endif
ifeq ($(shell expr $(ch) \>= 6), 1)
	AS_SRCS += $(TRAP)/kernelvec.S
endif

C_OBJS = $(addsuffix .o, $(basename $(C_SRCS)))
AS_OBJS = $(addsuffix .o, $(basename $(AS_SRCS)))
OBJS = $(C_OBJS) $(AS_OBJS)
HEADER_DEP = $(addsuffix .d, $(basename $(C_OBJS)))

-include $(HEADER_DEP)

ifeq ($(shell expr $(ch) \<= 5)$(shell expr $(ch) \>= 2), 11)
ifeq (,$(findstring link_app.o,$(OBJS)))
	AS_OBJS += $K/link_app.o
endif
endif

ifeq ($(shell expr $(ch) \>= 5), 1)
ifeq (,$(findstring initproc.o,$(OBJS)))
	AS_OBJS += $K/initproc.o
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

$(AS_OBJS): %.o : %.S
	@mkdir -p $(BUILDDIR)/$(@D)
	$(CC) $(CFLAGS) -c $< -o $@
	@cp $@ $(BUILDDIR)/$(@D)

$(C_OBJS): %.o : %.c %.d
	@mkdir -p $(BUILDDIR)/$(@D)
	$(CC) $(CFLAGS) -c $< -o $@
	@cp $@ $(BUILDDIR)/$(@D)

$(HEADER_DEP): %.d : %.c
	@mkdir -p $(BUILDDIR)/$(@D)
	@set -e; rm -f $@; $(CC) -MM $< $(INCLUDEFLAGS) > $@.$$$$; \
        sed 's,\($*\)\.o[ :]*,\1.o $@ : ,g' < $@.$$$$ > $@; \
        rm -f $@.$$$$
	@mv $@ $(BUILDDIR)/$(@D)

INIT_PROC ?= usershell

ifeq ($(shell expr $(ch) \>= 6), 1)
$(K)/initproc.o: $K/initproc.S
$(K)/initproc.S: $(SCRIPT)/initproc.py .FORCE
	cd $(SCRIPT) && $(PY) initproc.py $(INIT_PROC) $K

else ifeq ($(shell expr $(ch) \== 5), 1)
$(K)/initproc.o: $K/initproc.S
$(K)/initproc.S: $(SCRIPT)/initproc.py .FORCE
	cd $(SCRIPT) && $(PY) initproc.py $(INIT_PROC) $K

$(K)/link_app.o: $K/link_app.S
$(K)/link_app.S: $(SCRIPT)/pack.py .FORCE
	cd $(SCRIPT) && $(PY) pack.py $K
$(SCRIPT)/kernel_app.ld: $(SCRIPT)/kernelld.py .FORCE
	cd $(SCRIPT) && $(PY) kernelld.py

else ifeq ($(shell expr $(ch) \== 1), 1)

else
$K/link_app.o: $K/link_app.S
$K/link_app.S: $(SCRIPT)/pack.py .FORCE
	cd $(SCRIPT) && $(PY) pack.py $K
$(SCRIPT)/kernel_app.ld: $(SCRIPT)/kernelld.py .FORCE
	cd $(SCRIPT) && $(PY) kernelld.py
endif

build: build/kernel

ifeq ($(shell expr $(ch) \!= 1)$(shell expr $(ch) \!= 6)$(shell expr $(ch) \!= 7)$(shell expr $(ch) \!= 8), 1111)
build/kernel: $(OBJS) $(SCRIPT)/kernel_app.ld
	$(LD) $(LDFLAGS) -T $(SCRIPT)/kernel_app.ld -o $(BUILDDIR)/kernel $(OBJS)
	$(OBJDUMP) -S $(BUILDDIR)/kernel > $(BUILDDIR)/kernel.asm
	$(OBJDUMP) -t $(BUILDDIR)/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(BUILDDIR)/kernel.sym
	@rm -f $(OBJS) $(HEADER_DEP) $K/*.d $(ASM)/*.d $(TRAP)/*.d
	@echo 'Build kernel done'

else
build/kernel: $(OBJS) $(SCRIPT)/kernel.ld
	$(LD) $(LDFLAGS) -T $(SCRIPT)/kernel.ld -o $(BUILDDIR)/kernel $(OBJS)
	$(OBJDUMP) -S $(BUILDDIR)/kernel > $(BUILDDIR)/kernel.asm
	$(OBJDUMP) -t $(BUILDDIR)/kernel | sed '1,/SYMBOL TABLE/d; s/ .* / /; /^$$/d' > $(BUILDDIR)/kernel.sym
	@rm -f $(OBJS) $(HEADER_DEP) $K/*.d $(ASM)/*.d $(TRAP)/*.d
	@echo 'Build kernel done'
endif

clean:
	rm -rf $(BUILDDIR)
	make -C $(U) clean
	make -C $(F) clean

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

