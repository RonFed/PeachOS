section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler

global int21h
global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper

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
    ; Push all general purpose regs
    pushad
    call int21h_handler
    popad
    iret

no_interrupt:
    ; Push all general purpose regs
    pushad
    call no_interrupt_handler
    popad
    iret

isr80h_wrapper:
    ; -----INTERRUPT FRAME START------
    ; ALRREADY PUSHED BY THE PROCESSOR UPON ENTRING THIS INTERRUPT :
    ;   uint32_t ip
    ;   uint32_t cs
    ;   uint32_t flags
    ;   uint32_t sp
    ;   uint32_t ss

    ; Push general purpose registers
    pushad
    ; -------INTERRUPT FRAME END--------

    ; Push the stck pointer so that we are pointing to the interrupt frame
    push esp

    ; Push the command code from user
    push eax
    call isr80h_handler
    mov dword[tmp_res], eax
    add esp, 8

    ; Restore general purpose registers from user land
    popad
    mov eax, [tmp_res]
    iretd

section .data
; Inside here is stored the reutned result from isr80_handler
tmp_res: dd 0