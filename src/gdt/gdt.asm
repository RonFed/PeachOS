section .asm
global gdt_load

gdt_load:
    mov eax, [esp + 4]
    mov [gdt_descriptor + 2], eax   ; struct gdt*
    mov ax, [esp + 8]
    mov [gdt_descriptor], ax        ; int size
    lgdt [gdt_descriptor]
    ret

section .data
gdt_descriptor:
    dw 0x00 ; Size
    dd 0x00 ; GDT start address