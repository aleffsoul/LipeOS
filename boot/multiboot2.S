/* GRUB Multiboot2 entry. GRUB enters in 32-bit protected mode; this
 * trampoline builds identity paging and switches into long mode. */
.set MB2_MAGIC, 0xe85250d6
.set MB2_ARCH, 0

.section .multiboot2,"a"
.balign 8
mb2_header:
    .long MB2_MAGIC
    .long MB2_ARCH
    .long mb2_header_end - mb2_header
    .long -(MB2_MAGIC + MB2_ARCH + (mb2_header_end - mb2_header))
    /* Required end tag. */
    .short 0
    .short 0
    .long 8
mb2_header_end:

.section .text.boot,"ax"
.code32
.global multiboot_entry
.type multiboot_entry, @function
.extern kernel_main
.extern __bss_start
.extern __bss_end

multiboot_entry:
    cli
    cld
    movl %eax, boot_magic_saved
    movl %ebx, boot_info_saved
    movl $boot_stack_top, %esp

    /* The initial tables identity-map the first GiB using 2 MiB pages. */
    movl $boot_pdpt, %eax
    orl $0x03, %eax
    movl %eax, boot_pml4
    movl $0, boot_pml4 + 4

    movl $boot_page_directory, %eax
    orl $0x03, %eax
    movl %eax, boot_pdpt
    movl $0, boot_pdpt + 4

    movl $boot_page_directory, %edi
    movl $0x00000083, %eax
    movl $512, %ecx
1:
    movl %eax, (%edi)
    movl $0, 4(%edi)
    addl $0x00200000, %eax
    addl $8, %edi
    loop 1b

    movl $boot_pml4, %eax
    movl %eax, %cr3

    /* Enable PAE, long mode, and paging. */
    movl %cr4, %eax
    orl $0x20, %eax
    movl %eax, %cr4

    movl $0xc0000080, %ecx      /* IA32_EFER */
    rdmsr
    orl $0x00000100, %eax       /* EFER.LME */
    wrmsr

    lgdt gdt64_pointer
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movl %cr0, %eax
    orl $0x80000001, %eax       /* PG | PE */
    movl %eax, %cr0
    ljmp $0x08, $long_mode_entry

.code64
long_mode_entry:
    movw $0x10, %ax
    movw %ax, %ds
    movw %ax, %es
    movw %ax, %ss
    movw %ax, %fs
    movw %ax, %gs

    movq $boot_stack_top, %rsp
    andq $-16, %rsp
    movl boot_magic_saved(%rip), %edi
    movl boot_info_saved(%rip), %esi
    call kernel_main
2:
    cli
    hlt
    jmp 2b

.size multiboot_entry, . - multiboot_entry

.section .rodata.boot,"a"
.balign 8
gdt64:
    .quad 0x0000000000000000
    .quad 0x00af9a000000ffff   /* 64-bit ring-0 code */
    .quad 0x00cf92000000ffff   /* ring-0 data */
gdt64_end:
gdt64_pointer:
    .word gdt64_end - gdt64 - 1
    .long gdt64

.section .bss.boot,"aw",@nobits
.balign 8
boot_magic_saved:
    .long 0
boot_info_saved:
    .long 0
.balign 4096
boot_pml4:
    .skip 4096
.balign 4096
boot_pdpt:
    .skip 4096
.balign 4096
boot_page_directory:
    .skip 4096
.balign 16
boot_stack:
    .skip 16384
boot_stack_top:

.section .note.GNU-stack,"",@progbits
