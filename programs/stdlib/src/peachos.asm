[BITS 32]

section .asm

global print:function
global peachos_getkey:function
global peachos_malloc:function
global peachos_free:function
global peachos_putchar:function

; void print(const char* str)
print:
    push ebp
    mov ebp, esp
    push dword[ebp+8]
    mov eax, 1; Command print
    int 0x80
    add esp, 4
    pop ebp
    ret

; int peachos_getkey()
peachos_getkey:
    push ebp
    mov ebp, esp
    mov eax, 2; Command getkey
    int 0x80
    pop ebp
    ret

; void peachos_putchar(char c)
peachos_putchar:
    push ebp
    mov ebp, esp
    push dword[ebp+8]; push c
    mov eax, 3; Command putchar
    int 0x80
    add esp, 4
    pop ebp
    ret

; void* peachos_malloc(size_t size)
peachos_malloc:
    push ebp
    mov ebp, esp
    push dword[ebp+8]; push size
    mov eax, 4; Command malloc
    int 0x80
    add esp, 4
    pop ebp
    ret

; void peachos_free(void* ptr)
peachos_free:
    push ebp
    mov ebp, esp
    push dword[ebp+8]; push ptr to free
    mov eax, 5; Command free
    int 0x80
    add esp, 4
    pop ebp
    ret
