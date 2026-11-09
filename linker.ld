OUTPUT_FORMAT(elf64-x86-64)
ENTRY(multiboot_entry)

PHDRS
{
    text   PT_LOAD FLAGS(5); /* R-X */
    rodata PT_LOAD FLAGS(4); /* R-- */
    data   PT_LOAD FLAGS(6); /* RW- */
}

SECTIONS
{
    . = 1M;

    .multiboot2 ALIGN(8) :
    {
        KEEP(*(.multiboot2))
    } :text

    .text ALIGN(4K) :
    {
        *(.text.boot)
        *(.text .text.*)
    } :text

    .rodata ALIGN(4K) :
    {
        *(.rodata.boot)
        *(.rodata .rodata.*)
    } :rodata

    .data ALIGN(4K) :
    {
        *(.data .data.*)
    } :data

    .bss ALIGN(4K) (NOLOAD) :
    {
        __bss_start = .;
        *(COMMON)
        *(.bss .bss.*)
        __bss_end = .;
    } :data

    /DISCARD/ :
    {
        *(.comment)
        *(.eh_frame .eh_frame.*)
        *(.note .note.*)
    }
}
