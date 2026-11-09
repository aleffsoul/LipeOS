#!/bin/sh
set -eu
cd "$(dirname "$0")/.."
for tool in gcc ld grub-mkrescue grub-file xorriso python3; do
    if ! command -v "$tool" >/dev/null 2>&1; then
        echo "[build] missing required tool: $tool" >&2
        exit 1
    fi
done
make clean
make all check
printf '\n[build] ISO ready: %s/dist/LipeOS.iso\n' "$PWD"
