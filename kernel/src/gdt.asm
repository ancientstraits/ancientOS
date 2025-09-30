global setup_gdt

%define MAKE_GDT_DESCRIPTOR_UPPER32(base, limit, access_byte, flags) ( \
        ((base >> 16) & 0x00FF) | (access_byte << 8) | (limit & 0xF0000) \
        | (flags << 20) | (base & 0xFF000000) \
    )

%define MAKE_GDT_DESCRIPTOR_LOWER32(base, limit, access_byte, flags) \
    ((limit & 0x0FFFF) | ((base << 16) & 0x0000FFFF))

%define MAKE_GDT_DESCRIPTOR(base, limit, access_byte, flags) ( \
        (MAKE_GDT_DESCRIPTOR_UPPER32(base, limit, access_byte, flags) << 32) \
        | MAKE_GDT_DESCRIPTOR_LOWER32(base, limit, access_byte, flags) \
    )

section .data

; gdt_print_start: db "GDT START",0
; gdt_print_end: db "GDT END",0

gdt_start:
    dq 0 ; the null descriptor
    gdt_code_offset equ $ - gdt_start
    dq MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0x9A, 0xA) ; kernel mode code segment
    gdt_data_offset equ $ - gdt_start
    dq MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0x92, 0xC) ; kernel mode data segment
    dq MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0xFA, 0xA) ; user   mode code segment
    dq MAKE_GDT_DESCRIPTOR(0, 0xFFFFF, 0xF2, 0xC) ; user   mode data segment
gdt_end:

gdt_size equ gdt_end - gdt_start

gdtr: ; gdt register
    dw gdt_size ; limit
    dq gdt_start ; base address


section .text

setup_gdt:
    cli ; disable interrupts

    lgdt [gdtr]

    ; reload segments
    ; Intel says:
    ;    When executing a far return, the processor pops the return instruction pointer
    ;    from the top of the stack into the EIP register, then pops the segment selector
    ;    from the top of the stack into the CS register.
    ;    The processor then begins program execution in the new code segment at the new instruction pointer.
    ; That means that RAX (the relative .reload address) is put into the EIP register, and
    ; the gdt code offset is put into the CS register, so this lets us set the CS register.
    push gdt_code_offset
    lea rax, [rel .reload]
    push rax
    retfq
.reload:
    mov ax, gdt_data_offset ; We need to set all these registers to the data offset
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ret

; print_gdt:
;     mov rdi, gdt_print_start
;     call puts
;     mov rdx, 6
; .loop:
;     mov rdi, rdx
;     call putlong
;     add rdx, 1

;     cmp rdx, 10
;     je .end
;     jmp .loop
; .end:
;     mov rdi, gdt_print_end
;     call puts

