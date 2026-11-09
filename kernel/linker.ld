OUTPUT_FORMAT(elf32-i386)
ENTRY(_start)
PHDRS
{
  text PT_LOAD FLAGS(5); /* read + execute */
  data PT_LOAD FLAGS(6); /* read + write */
}
SECTIONS
{
  . = 0x00010000;
  .text : ALIGN(16) { KEEP(*(.text.start)) *(.text*) } :text
  .rodata : ALIGN(16) { *(.rodata*) } :text
  .data : ALIGN(16) { *(.data*) } :data
  __kernel_file_end = .;
  .bss : ALIGN(16) {
    __bss_start = .;
    *(COMMON) *(.bss*)
    __bss_end = .;
  } :data
  /DISCARD/ : { *(.comment) *(.note*) *(.eh_frame*) }
}
