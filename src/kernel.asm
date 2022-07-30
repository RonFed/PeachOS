[BITS 32]
global _start
extern kernel_main

CODE_SEG equ 0x08
DATA_SEG equ 0X10

_start:
    mov ax, DATA_SEG
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax
    mov ebp, 0x00200000
    mov esp, ebp

    ; Enable A20 line
    in al, 0x92
    or al, 2
    out 0x92, al


    ; Remap the master Programmable interrupt controller (PIC)
    mov al, 00010001b
    out 0x20, al ; Tell Master PIC

    mov al, 0x20 ; Interrupt 0x20 is where the master isr should start
    out 0x21, al

    mov al, 00000001b
    out 0x21, al
    ; End remap of master PIC

    call kernel_main
    jmp $


times 512-($ -$$) db 0