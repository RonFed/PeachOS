section .asm

extern int21h_handler
extern no_interrupt_handler
extern isr80h_handler
extern interrupt_handler

global idt_load
global no_interrupt
global enable_interrupts
global disable_interrupts
global isr80h_wrapper
global interrupt_pointer_table

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

no_interrupt:
    ; Push all general purpose regs
    pushad
    call no_interrupt_handler
    popad
    iret

; General interrupt macro: get one argument which is the interrupt number
%macro interrupt 1
    global int%1
    int%1:
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
        push esp
        push dword %1
        call interrupt_handler
        add esp, 8
        popad
        iret
%endmacro

%assign i 0
%rep 512
    interrupt i
    %assign i i+1
%endrep


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
; Inside here is stored the returned result from isr80_handler
tmp_res: dd 0

%macro interrupt_array_entry 1
    dd int%1 ; Store interrupt i address
%endmacro

interrupt_pointer_table:
    %assign i 0
    %rep 512
        interrupt_array_entry i 
        %assign i i+1
    %endrep