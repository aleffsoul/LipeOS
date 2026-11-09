# LipeOS

Kernel mínimo **x86_64** para testes em VM. O boot usa **GRUB2 + Multiboot2**; não requer outro sistema operacional dentro da VM. O kernel atual é C freestanding com Assembly de entrada, transição de 32 para 64 bits, GDT, IDT, PIC/PIT, teclado PS/2, console VGA texto, serial COM1 e shell básico.

> **Estado:** primeira versão de desenvolvimento. O kernel ELF compila e o cabeçalho Multiboot2 passa a validação automatizada local. Esta cópia de trabalho não tem GRUB/xorriso nem QEMU instalados, então a geração da ISO e o boot QEMU ainda não foram executados aqui. Não considerar validado para usuário final até `make test` passar num host com os requisitos abaixo.

## Estrutura

- `boot/multiboot2.S` — cabeçalho Multiboot2, GDT e transição para long mode.
- `kernel/` — entry IRQ e inicialização do IDT/shell.
- `drivers/` — COM1, VGA, PIC, PIT e teclado PS/2.
- `include/kernel.h` — interface mínima compartilhada.
- `linker.ld` — layout ELF64 a partir de 1 MiB.
- `grub.cfg` — entrada de boot GRUB2.
- `tests/` — validação ELF/Multiboot2, ISO e smoke test QEMU.
- `Dockerfile` — ambiente reproduzível com toolchain, GRUB, xorriso e QEMU.

## Requisitos de build

Em Debian/Ubuntu (ou WSL2 Ubuntu):

```sh
sudo apt update
sudo apt install build-essential gcc binutils make python3 \
  grub-common grub-pc-bin xorriso mtools qemu-system-x86 coreutils
```

Não é necessário cross-compiler: GCC/binutils x86_64 do host geram o kernel freestanding. O ISO BIOS requer os módulos GRUB PC e `xorriso`.

## Compilar e testar

Build limpo, kernel ELF, ISO e verificações estruturais:

```sh
./scripts/build.sh
```

Comandos equivalentes:

```sh
make clean all check
```

Como teste complementar (não substitui QEMU), pode-se usar Unicorn: `python3 -m pip install unicorn && make test-unicorn`.

Executar também o teste de boot headless no QEMU:

```sh
make test
```

O teste inicia QEMU por alguns segundos, captura COM1 e exige o banner de sucesso e a inicialização das interrupções. A ISO final fica em **`dist/LipeOS.iso`**.

## Executar a VM

Com console gráfico do QEMU e COM1 ligado ao terminal:

```sh
qemu-system-x86_64 -cdrom dist/LipeOS.iso -m 256M -serial stdio -no-reboot
```

Ou:

```sh
./scripts/run-qemu.sh
```

Para smoke test headless: `make test-qemu` usa `-display none` e grava COM1 em `build/qemu-serial.log`.

### Shell

Teclado PS/2 (layout QWERTY básico): `help`, `clear`, `about`, `reboot`. O banner inicial em VGA e COM1 é `LipeOS iniciado com sucesso`.

## Windows

### WSL2 (recomendado)

1. No PowerShell como administrador: `wsl --install -d Ubuntu`; reinicie se solicitado.
2. Abra Ubuntu e instale os pacotes da seção **Requisitos de build**.
3. Clone/abra este repositório dentro do filesystem Linux, por exemplo `~/src/LipeOS` (evite `/mnt/c` para builds mais rápidos).
4. Execute `./scripts/build.sh`; a ISO aparecerá em `dist/LipeOS.iso`.
5. Para QEMU, instale `qemu-system-x86` no Ubuntu e use `./scripts/run-qemu.sh`.

### Docker Desktop

Com Docker Desktop usando backend WSL2:

```powershell
docker build -t lipeos-builder .
docker run --rm -v "${PWD}:/src" lipeos-builder
```

O container executa build, verificações e smoke test headless. `dist/LipeOS.iso` é criado no diretório do repositório no Windows. Se a montagem do PowerShell não funcionar, use WSL2 e passe o caminho Linux, por exemplo `-v "${PWD}:/src"` dentro de uma shell WSL.

## Limpeza

```sh
./scripts/clean.sh
```

Remove `build/` e `dist/LipeOS.iso`.

## Limitações desta versão

O kernel ainda usa mapeamento identidade de 1 GiB com páginas de 2 MiB, modo texto VGA herdado do GRUB e apenas controlador PS/2 básico. Não há modo gráfico próprio, gerenciamento geral de memória, processos, isolamento, sistema de arquivos, chamadas de sistema, rede ou USB. `reboot` usa o controlador de teclado legado. Esses itens são melhorias futuras e não devem alterar o caminho de boot básico sem testes de regressão.
