ORG 0x7c00
BITS 16

CODE_SEG equ gdt_code - gdt_start ; 0x8
DATA_SEG equ gdt_data - gdt_start

_start:
    jmp short start
    nop

times 33 db 0 ; Add space for BIOS paramater block in case it'll write here

start:
    jmp 0:step2

step2:
    cli ; Clear interrupts
    mov ax, 0x00
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7c00
    sti ; Enable interrupts 

.load_protected:
    cli
    lgdt[gdt_descriptor]
    mov eax, cr0
    or eax, 0x1
    mov cr0, eax
    jmp CODE_SEG:load32
    jmp $

; GDT
gdt_start:
gdt_null:
    dd 0x0
    dd 0x0

; offset 0x8
gdt_code:       ; CS should point to this
    dw 0xffff   ; Segment limit first 0-15 bits
    dw 0        ; Base first 0-15 bits
    db 0        ; Base 16-23 bits
    db 0x9a     ; Access byte (flags for prmission, kernel, executable, ..)
    db 11001111b; High 4 bit flags and low 4 bit flags
    db 0        ; Base 24-31 bits

; offset 0x10
gdt_data:       ; DS, SS, ES, FS, GS
    dw 0xffff   ; Segment limit first 0-15 bits
    dw 0        ; Base first 0-15 bits
    db 0        ; Base 16-23 bits
    db 0x92     ; Access byte (flags for prmission, kernel, executable, ..)
    db 11001111b; High 4 bit flags and low 4 bit flags
    db 0        ; Base 24-31 bits

gdt_end:

gdt_descriptor:
    dw gdt_end - gdt_start - 1
    dd gdt_start

[BITS 32]
load32:
    mov eax, 1  ; The sector we want to start from (0 is boot sector)
    mov ecx, 100; Total number of sectors to load
    mov edi, 0x0100000 ; The address to load to
    call ata_lta_read
    jmp CODE_SEG:0x100000

ata_lta_read:
    mov ebx, eax ;Backup LBA
    ; Send the highest 8 bits of the LBA to the hard disk controller
    shr eax, 24
    or eax, 0xE0 ; Select the master drive
    mov dx, 0x1F6
    out dx, al
    ; FInisehd sending the highhest 8 bits of LBA

    ; Send total sectors to read
    mov eax, ecx
    mov dx, 0x1F2
    out dx, al
    ; FInished sending the total sectors to read

    mov eax, ebx ; Restoring the backup LBA
    mov dx, 0x1F3
    out dx, al

    mov dx, 0x1F4
    mov eax, ebx
    shr eax, 8
    out dx, al

    mov dx, 0x1F5
    mov eax, ebx
    shr eax, 16
    out dx, al

    mov dx, 0x1F7
    mov al, 0x20
    out dx, al

    ; Read all sectors into memory
.next_sector:
    push ecx

; Checking if we need to read
.try_again:
    mov dx, 0x1F7
    in al, dx
    test al, 8
    jz .try_again

; Read 256 words at a time
    mov ecx, 256
    mov dx, 0x1F0
    rep insw ; Read 256 words = 512 bytes = 1 sector
    pop ecx
    loop .next_sector
    ; ENd of reading sectors into memory
    ret


# pad this code with 0 bytes to have 510 bytes long
times 510-($ -$$) db 0
# add boot loader signature as the last two bytes
dw 0xAA55
