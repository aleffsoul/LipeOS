#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
if ! command -v qemu-system-x86_64 >/dev/null 2>&1; then
    echo "[test-qemu] qemu-system-x86_64 is required." >&2
    exit 127
fi
if ! command -v timeout >/dev/null 2>&1; then
    echo "[test-qemu] GNU timeout is required." >&2
    exit 127
fi
log=build/qemu-serial.log
mkdir -p build
rm -f "$log"
set +e
timeout 12s qemu-system-x86_64 \
    -display none \
    -cdrom dist/LipeOS.iso \
    -m 256M \
    -serial "file:$log" \
    -no-reboot
status=$?
set -e
if [ "$status" -ne 0 ] && [ "$status" -ne 124 ] && [ "$status" -ne 137 ]; then
    echo "[test-qemu] QEMU exited with status $status" >&2
    cat "$log" >&2 || true
    exit "$status"
fi
if ! grep -Fq "LipeOS iniciado com sucesso" "$log"; then
    echo "[test-qemu] boot banner not found in serial output" >&2
    cat "$log" >&2 || true
    exit 1
fi
if ! grep -Fq "[interrupts] PIC remapped" "$log"; then
    echo "[test-qemu] interrupt initialization log missing" >&2
    cat "$log" >&2 || true
    exit 1
fi
echo "[test-qemu] PASS: QEMU reached the LipeOS banner and initialized interrupts"
