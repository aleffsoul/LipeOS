.section .text
.code64

.macro SAVE_GPRS
    pushq %rax
    pushq %rbx
    pushq %rcx
    pushq %rdx
    pushq %rsi
    pushq %rdi
    pushq %rbp
    pushq %r8
    pushq %r9
    pushq %r10
    pushq %r11
    pushq %r12
    pushq %r13
    pushq %r14
    pushq %r15
    movq %rsp, %r12
    andq $-16, %rsp
.endm

.macro RESTORE_GPRS
    movq %r12, %rsp
    popq %r15
    popq %r14
    popq %r13
    popq %r12
    popq %r11
    popq %r10
    popq %r9
    popq %r8
    popq %rbp
    popq %rdi
    popq %rsi
    popq %rdx
    popq %rcx
    popq %rbx
    popq %rax
.endm

.global isr_exception
.type isr_exception, @function
isr_exception:
    cli
    cld
    SAVE_GPRS
    call exception_handler
1:  hlt
    jmp 1b

.global isr_default
.type isr_default, @function
isr_default:
    cld
    SAVE_GPRS
    call unhandled_irq_handler
    RESTORE_GPRS
    iretq

.global isr_timer
.type isr_timer, @function
isr_timer:
    cld
    SAVE_GPRS
    call timer_irq_handler
    RESTORE_GPRS
    iretq

.global isr_keyboard
.type isr_keyboard, @function
isr_keyboard:
    cld
    SAVE_GPRS
    call keyboard_irq_handler
    RESTORE_GPRS
    iretq

.section .note.GNU-stack,"",@progbits
