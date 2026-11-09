#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "[run] qemu-system-x86_64 is required (package: qemu-system-x86)." >&2
    exit 127
fi
exec qemu-system-x86_64 -cdrom dist/LipeOS.iso -m 256M -serial stdio -no-reboot
