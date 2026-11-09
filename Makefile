CC = gcc
LD = ld
GRUB_MKRESCUE ?= grub-mkrescue
GRUB_FILE ?= grub-file
PYTHON ?= python3
BUILD := build
ISO_ROOT := $(BUILD)/iso-root
KERNEL := $(BUILD)/lipeos.elf

CFLAGS := -std=gnu11 -O2 -Wall -Wextra -Werror \
          -ffreestanding -fno-builtin -fno-stack-protector \
          -fno-pic -fno-pie -fno-asynchronous-unwind-tables \
          -fno-unwind-tables -m64 -mcmodel=small -mno-red-zone \
          -mno-sse -mno-sse2 -mno-mmx -msoft-float \
          -Iinclude
ASFLAGS := -m64 -ffreestanding -fno-pic -fno-pie -Iinclude
LDFLAGS := -m elf_x86_64 -T linker.ld -nostdlib -z max-page-size=0x1000
C_SOURCES := kernel/main.c kernel/idt.c drivers/serial.c drivers/vga.c \
             drivers/pic.c drivers/pit.c drivers/keyboard.c
ASM_SOURCES := boot/multiboot2.S kernel/interrupts.S
OBJECTS := $(patsubst %.c,$(BUILD)/%.o,$(C_SOURCES)) \
           $(patsubst %.S,$(BUILD)/%.o,$(ASM_SOURCES))

.PHONY: all kernel iso check check-kernel test test-qemu test-unicorn run clean
all: iso

kernel: $(KERNEL)

iso: dist/LipeOS.iso

$(BUILD)/%.o: %.c
	@mkdir -p $(dir $@)
	@echo "[CC] $<"
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD)/%.o: %.S
	@mkdir -p $(dir $@)
	@echo "[AS] $<"
	$(CC) $(ASFLAGS) -c $< -o $@

$(KERNEL): $(OBJECTS) linker.ld
	@echo "[LD] $@"
	$(LD) $(LDFLAGS) -o $@ $(OBJECTS)

$(ISO_ROOT)/boot/lipeos.elf: $(KERNEL) grub.cfg
	@mkdir -p $(ISO_ROOT)/boot/grub
	cp $(KERNEL) $@
	cp grub.cfg $(ISO_ROOT)/boot/grub/grub.cfg

dist/LipeOS.iso: $(ISO_ROOT)/boot/lipeos.elf
	@mkdir -p dist
	@echo "[ISO] $@"
	$(GRUB_MKRESCUE) -o $@ $(ISO_ROOT)

check-kernel: $(KERNEL)
	$(PYTHON) tests/check_kernel.py $(KERNEL)
	@if command -v $(GRUB_FILE) >/dev/null 2>&1; then $(GRUB_FILE) --is-x86-multiboot2 $(KERNEL); else echo "[check] grub-file unavailable; Python Multiboot2 checks passed"; fi

check: dist/LipeOS.iso check-kernel
	$(PYTHON) tests/check_iso.py dist/LipeOS.iso
	$(GRUB_FILE) --is-x86-multiboot2 $(KERNEL)

# Requires qemu-system-x86_64 and GNU timeout.
test-qemu: check
	./tests/test_qemu.sh

test: test-qemu

test-unicorn: $(KERNEL)
	$(PYTHON) tests/test_kernel_unicorn.py

run: dist/LipeOS.iso
	./scripts/run-qemu.sh

clean:
	@echo "[CLEAN] removing build outputs"
	rm -rf $(BUILD) dist/LipeOS.iso
