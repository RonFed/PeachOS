section .asm

extern int21h_handler
extern no_interrupt_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts

enable_interrupts:
    sti
    ret

disable_interrupts:
    cli
    ret

idt_load:
    push ebp
    mov ebp, esp
    mov ebx, [ebp + 8]
    lidt [ebx]
    pop ebp
    ret

; Keyboard interrupt
int21h:
    ; Clear interrupts
    cli
    ; Push all general purpose regs
    pushad
    call int21h_handler
    popad
    sti
    iret

no_interrupt:
    ; Clear interrupts
    cli
    ; Push all general purpose regs
    pushad
    call no_interrupt_handler
    popad
    sti
    iret
