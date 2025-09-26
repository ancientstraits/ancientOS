global putbyte_asm

%define QEMU_DEBUGCON 0xE9

putbyte_asm:
    mov al, '0'
    out QEMU_DEBUGCON, al
    mov al, 'x'
    out QEMU_DEBUGCON, al

    mov al, dil
    shr al, 4 ; get upper 4 bits

    cmp al, 10
    mov dl, '0' ; dl will have the offset to convert the nibble
    mov cl, ('A' - 10)
    cmovge rdx, rcx

    add al, dl
    out QEMU_DEBUGCON, al

    mov al, dil
    and al, 0xF ; get lower 4 bits

    cmp al, 10
    mov dl, '0' ; dl will have the offset to convert the nibble
    mov cl, ('A' - 10)
    cmovge rdx, rcx

    add al, dl
    out QEMU_DEBUGCON, al

    mov al, 10
    out QEMU_DEBUGCON, al

    ret
