global setup_idt
global idt_set_descriptor
extern putbyte

struc idt_descriptor
    isr_low:  resw 1
    seg_sel:  resw 1
    ist:      resb 1
    attr:     resb 1
    isr_mid:  resw 1
    isr_high: resd 1
    reserved: resd 1
endstruc

%define MAKE_IDT_DESCRIPTOR_LOWER64(isr, seg_sel, ist, attr) \
    (((isr) & 0xFFFF) | ((seg_sel) << 16) | ((ist) << 32) | ((attr) << 40) | (((isr) >> 16) & 0xFFFF))

%define MAKE_IDT_DESCRIPTOR_UPPER64(isr, seg_sel, ist, attr) \
    ((isr) >> 32)

%define MAKE_IDT_DESCRIPTOR(isr, seg_sel, ist, attr) \
    dq MAKE_IDT_DESCRIPTOR_LOWER64(isr, seg_sel, ist, attr), MAKE_IDT_DESCRIPTOR_UPPER64(isr, seg_sel, ist, attr)

%define IDT_LIMIT(size) ((16 * (size)) - 1)

%macro isr_err_stub 1
isr_stub_%+%1:
    mov dil, %1
    call putbyte
    call exception_handler
    iret
%endmacro
%macro isr_no_err_stub 1
isr_stub_%+%1:
    mov dil, %1
    call putbyte
    call exception_handler
    iret
%endmacro

section .data

idt_start:
    times 256 dq 0, 0
idt_end:

idtr:
    dw IDT_LIMIT(256) ; limit
    dq idt_start ; base

section .text

exception_handler:
    mov al, 'E'
    out 0xE9, al
    cli
    hlt

isr_stubs:
    isr_no_err_stub 0
    isr_no_err_stub 1
    isr_no_err_stub 2
    isr_no_err_stub 3
    isr_no_err_stub 4
    isr_no_err_stub 5
    isr_no_err_stub 6
    isr_no_err_stub 7
    isr_err_stub    8
    isr_no_err_stub 9
    isr_err_stub    10
    isr_err_stub    11
    isr_err_stub    12
    isr_err_stub    13
    isr_err_stub    14
    isr_no_err_stub 15
    isr_no_err_stub 16
    isr_err_stub    17
    isr_no_err_stub 18
    isr_no_err_stub 19
    isr_no_err_stub 20
    isr_no_err_stub 21
    isr_no_err_stub 22
    isr_no_err_stub 23
    isr_no_err_stub 24
    isr_no_err_stub 25
    isr_no_err_stub 26
    isr_no_err_stub 27
    isr_no_err_stub 28
    isr_no_err_stub 29
    isr_err_stub    30
    isr_no_err_stub 31

isr_stub_table:
    %assign i 0
    %rep 32
        dq isr_stub_%+i
        %assign i i+1
    %endrep

; void idt_set_descriptor(uint8_t idx, void* isr, uint8_t attrs);
; dil = idx, rsi = isr, dl = attrs
idt_set_descriptor:
    movzx rdi, dil ; zero-extend
    shl rdi, 4 ; multiply by 16 to get correct offset (each idt is 16 bytes)
    mov word  idt_start[rdi + 0], si   ; isr_low
    shr rsi, 16 ; shift right 16 bits
    mov word  idt_start[rdi + 2], 0x08 ; the value of `gdt_code_offset`
    mov byte  idt_start[rdi + 4], 0    ; zero the idt field
    mov byte  idt_start[rdi + 5], dl   ; attrs
    mov qword idt_start[rdi + 6], rsi  ; I hope I can set isr_mid and isr_high at the same time...
    mov dword idt_start[rdi + 12], 0   ; zero the upper 32 bits

    ret

; void setup_idt(void);
setup_idt:
    push rbx

    mov rbx, 0
    mov dl, 0b10001110
.loop: ; assign 32 stubs
    mov rdi, rbx
    mov rsi, qword isr_stub_table[8 * rdi] ; move to isr_stub_(rdi)
    call idt_set_descriptor

    inc rbx
    cmp rbx, 32 ; do this 32 times
    jl .loop
.end:
    pop rbx

    lidt [idtr]

    ret
