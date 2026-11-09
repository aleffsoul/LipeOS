#!/usr/bin/env python3
"""Check ISO9660 descriptors and the El Torito BIOS boot catalog."""
from pathlib import Path
import sys

BLOCK = 2048
path = Path(sys.argv[1] if len(sys.argv) > 1 else "dist/LipeOS.iso")
data = path.read_bytes()
if len(data) % BLOCK:
    raise SystemExit("ISO size is not a multiple of 2048 bytes")

def sector(lba: int) -> bytes:
    start = lba * BLOCK
    return data[start:start + BLOCK]

pvd = sector(16)
if pvd[:7] != b"\x01CD001\x01":
    raise SystemExit("ISO9660 primary volume descriptor missing at sector 16")
boot_record = sector(17)
if boot_record[:7] != b"\x00CD001\x01" or b"EL TORITO SPECIFICATION" not in boot_record[7:39]:
    raise SystemExit("El Torito boot record missing at sector 17")
if sector(18)[:7] != b"\xffCD001\x01":
    raise SystemExit("ISO volume descriptor terminator missing")

catalog_lba = int.from_bytes(boot_record[71:75], "little")
catalog = sector(catalog_lba)
if catalog[0] != 1 or catalog[30:32] != b"\x55\xaa":
    raise SystemExit("invalid El Torito validation entry")
checksum = sum(int.from_bytes(catalog[i:i + 2], "little") for i in range(0, 32, 2)) & 0xffff
if checksum:
    raise SystemExit("El Torito validation checksum failed")
if catalog[32] != 0x88:
    raise SystemExit("El Torito default boot entry is not bootable")
if not any(catalog[32:64]):
    raise SystemExit("empty El Torito default boot entry")
print(f"ISO OK: {path} ({len(data)} bytes), ISO9660 + El Torito BIOS boot catalog")
