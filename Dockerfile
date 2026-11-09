FROM debian:bookworm-slim

RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential gcc binutils make python3 \
    grub-common grub-pc-bin xorriso mtools \
    qemu-system-x86 coreutils \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /src
CMD ["sh", "-lc", "make clean all check test-qemu"]
