#!/usr/bin/env python3
"""Validate ELF64 and Multiboot2 header without external GRUB utilities."""
from pathlib import Path
import struct
import sys

path = Path(sys.argv[1] if len(sys.argv) > 1 else "build/lipeos.elf")
data = path.read_bytes()
if len(data) < 64 or data[:4] != b"\x7fELF":
    raise SystemExit("kernel is not an ELF file")
if data[4] != 2 or data[5] != 1:
    raise SystemExit("kernel must be ELF64 little-endian")
if struct.unpack_from("<H", data, 18)[0] != 62:
    raise SystemExit("ELF machine is not x86_64")
entry = struct.unpack_from("<Q", data, 24)[0]
if not 0x100000 <= entry < 0x40000000:
    raise SystemExit(f"unexpected kernel entry address: {entry:#x}")

magic = 0xe85250d6
limit = min(len(data), 32768)
header_offset = None
for offset in range(0, limit - 16 + 1, 8):
    if struct.unpack_from("<I", data, offset)[0] == magic:
        header_offset = offset
        break
if header_offset is None:
    raise SystemExit("Multiboot2 header not found in first 32 KiB")
header_magic, architecture, length, checksum = struct.unpack_from("<IIII", data, header_offset)
if architecture != 0:
    raise SystemExit("Multiboot2 header must target i386 protected-mode entry")
if length < 16 or header_offset + length > limit:
    raise SystemExit("invalid Multiboot2 header length")
if (header_magic + architecture + length + checksum) & 0xffffffff:
    raise SystemExit("Multiboot2 header checksum is invalid")
tags = data[header_offset + 16:header_offset + length]
if tags[:8] != b"\0\0\0\0\x08\0\0\0":
    raise SystemExit("Multiboot2 end tag is missing or malformed")
print(f"kernel OK: ELF64 x86_64, entry={entry:#x}, Multiboot2 header at file offset {header_offset}")
