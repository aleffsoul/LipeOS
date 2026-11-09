#!/usr/bin/env python3
"""Optional x86_64 CPU-level smoke test; this is not a GRUB/QEMU boot test."""
from pathlib import Path
import struct
import subprocess

try:
    from unicorn import Uc, UC_ARCH_X86, UC_MODE_64, UC_HOOK_CODE, UC_HOOK_INSN
    from unicorn.x86_const import (
        UC_X86_INS_IN, UC_X86_INS_OUT, UC_X86_REG_GDTR, UC_X86_REG_CS,
        UC_X86_REG_DS, UC_X86_REG_ES, UC_X86_REG_SS, UC_X86_REG_EFLAGS,
        UC_X86_REG_RSP, UC_X86_REG_RDI, UC_X86_REG_RIP,
    )
except ImportError as exc:
    raise SystemExit("Optional test dependency missing: python3 -m pip install unicorn") from exc

ELF = Path("build/lipeos.elf").read_bytes()

def get_symbols() -> dict[str, int]:
    symbols = {}
    for line in subprocess.check_output(["nm", "-an", "build/lipeos.elf"], text=True).splitlines():
        fields = line.split()
        if len(fields) >= 3:
            try:
                symbols[fields[-1]] = int(fields[0], 16)
            except ValueError:
                pass
    return symbols


SYMS = get_symbols()
uc = Uc(UC_ARCH_X86, UC_MODE_64)
uc.mem_map(0, 0x200000)
phoff = struct.unpack_from("<Q", ELF, 32)[0]
phentsize = struct.unpack_from("<H", ELF, 54)[0]
phnum = struct.unpack_from("<H", ELF, 56)[0]
for index in range(phnum):
    p_type, _flags, offset, vaddr, _paddr, filesz, memsz, _align = struct.unpack_from(
        "<IIQQQQQQ", ELF, phoff + index * phentsize
    )
    if p_type == 1:
        if filesz:
            uc.mem_write(vaddr, ELF[offset:offset + filesz])
        if memsz > filesz:
            uc.mem_write(vaddr + filesz, bytes(memsz - filesz))

# Supply a valid synthetic GRUB handoff, then start at the 64-bit entry label.
mbi = 0x180000
uc.mem_write(mbi, struct.pack("<IIQ", 16, 0, 0))
uc.mem_write(SYMS["boot_magic_saved"], (0x36d76289).to_bytes(4, "little"))
uc.mem_write(SYMS["boot_info_saved"], mbi.to_bytes(4, "little"))
uc.reg_write(UC_X86_REG_GDTR, (0, SYMS["gdt64"], SYMS["gdt64_end"] - SYMS["gdt64"] - 1, 0))
uc.reg_write(UC_X86_REG_CS, 0x08)
uc.reg_write(UC_X86_REG_DS, 0x10)
uc.reg_write(UC_X86_REG_ES, 0x10)
uc.reg_write(UC_X86_REG_SS, 0x10)
uc.reg_write(UC_X86_REG_EFLAGS, 2)

serial = bytearray()
stopped = []

def port_in(_uc, port, _size, _user_data):
    return 0x20 if port == 0x3fd else 0

def port_out(_uc, port, _size, value, _user_data):
    if port == 0x3f8:
        serial.append(value & 0xff)

def stop_at_halt(engine, address, _size, _user_data):
    if engine.mem_read(address, 1) == b"\xf4":
        stopped.append(address)
        engine.emu_stop()

uc.hook_add(UC_HOOK_INSN, port_in, None, 1, 0, UC_X86_INS_IN)
uc.hook_add(UC_HOOK_INSN, port_out, None, 1, 0, UC_X86_INS_OUT)
uc.hook_add(UC_HOOK_CODE, stop_at_halt)
uc.reg_write(UC_X86_REG_RSP, SYMS["boot_stack_top"] - 8)
uc.emu_start(SYMS["long_mode_entry"], 0xffffffff, count=10_000_000)
if not stopped:
    raise SystemExit("kernel did not reach its idle HLT loop")
log = bytes(serial)
for expected in (
    b"valid GRUB handoff",
    b"256 interrupt gates installed",
    b"PIC remapped",
    b"LipeOS iniciado com sucesso",
):
    if expected not in log:
        raise SystemExit(f"missing kernel smoke-test serial output: {expected!r}\n{log!r}")

# The VGA text buffer stores character/attribute pairs.
video = bytes(uc.mem_read(0xb8000, 80 * 25 * 2))
characters = video[0::2]
if b"LipeOS iniciado com sucesso" not in characters:
    raise SystemExit("success banner missing from the VGA text buffer")

print("PASS: x86_64 kernel entry, synthetic Multiboot2 handoff, IDT/PIC/PIT init, COM1 and VGA banner")
print("NOTE: this test does not execute GRUB, the 32-to-64-bit trampoline, a BIOS, or QEMU devices")
