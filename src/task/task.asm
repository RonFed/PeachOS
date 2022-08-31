[BITS 32]

section .asm

global restore_general_purpose_registers
global task_return
global user_registers

task_return:
    mov ebp, esp
    ; These pushes are what the cpu will normally push when interrupt is happening
        ; Push the data segment
        ; Push the stack address
        ; Push the flags
        ; Push the code segment
        ; Push Instruction Pointer

    ; Access the structure passed to us
    mov ebx, [ebp+4]
    ; Push the data/stack selector
    push dword [ebx+44]
    ; Push the stack pointer
    push dword [ebx+40]
    ; Push the flags
    pushf
    pop eax
    ; Enable interrupts for iret
    or eax, 0x200
    push eax
    ; Push the code segment
    push dword [ebx+32]
    ; Push the IP to execute (this is virtual address since paging is enabled)
    push dword [ebx+28]
    ; Setup segments registers
    mov ax, [ebx+44]
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax

    ; Restore general purpose registers
    push dword [ebp+4]
    call restore_general_purpose_registers
    add esp, 4
    ; Leave kernel land and execute in user land
    iretd

; void restore_general_purpose_registers(struct registers* regs)
restore_general_purpose_registers:
    push ebp
    mov ebp, esp
    ; get regs pointer to ebx
    mov ebx, [ebp+8]
    ; restore registers
    mov edi, [ebx]
    mov esi, [ebx+4]
    mov ebp, [ebx+8]
    mov edx, [ebx+16]
    mov ecx, [ebx+20]
    mov eax, [ebx+24]
    mov ebx, [ebx+12]

    pop ebp
    ret

user_registers:
    ; Change to User data segment (offset in gdt)
    mov ax, 0x23
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    ret