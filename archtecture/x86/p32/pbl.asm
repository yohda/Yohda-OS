%ifndef _PBL_NASM_
%define _PBL_NASM_

[BITS 32]

extern CPUID_PAGE_4MB
extern higher_half_start

MB_MAGIC_YOHDA_OS	equ 0x3f221d73
MB_MAGIC_MACH_STATE equ 0x36d76289

%ifdef GRUB
section .multiboot_header write
MB_TAG_TYPE_ADDRESS equ 0x2
MB_TAG_TYPE_ENTRY	equ 0x3

MULTIBOOT2_MAGIC equ 0xE85250D6
MULTIBOOT2_ARCH_I386 equ 0
MULTIBOOT2_ARCH_MIPS equ 4

;align 8
multiboot_header_start:
magic_fields_start:
	magic:			dd MULTIBOOT2_MAGIC 	; MAGIC NUMBER
	archtecture : 	dd MULTIBOOT2_ARCH_I386 ; i386 = 0 , MIPS = 4
	header_length:	dd multiboot_header_end - multiboot_header_start ; header length 
	checksum:		dd -(MULTIBOOT2_MAGIC + MULTIBOOT2_ARCH_I386 + (multiboot_header_end - multiboot_header_start)) 
magic_fields_end:
address_tag_start:
	dw MB_TAG_TYPE_ADDRESS								; type
	dw 1												; flag
	dd address_tag_end - address_tag_start				; size
	dd multiboot_header_start							; header_addr
	;dd _start 											; load_addr
	dd multiboot_header_start
	dd 0												; load_end_addr
	dd 0												; bss_end_addr
address_tag_end:
entry_address_tag_start:
	dw MB_TAG_TYPE_ENTRY								; type
	dw 1												; flag
	dd entry_address_tag_end - entry_address_tag_start	; size
	dd _start											; entry_address
entry_address_tag_end:
multiboot_header_end:
%endif ; GRUB

section .bss
align 16
stack_bottom:
resb 1024*32 ; 32KB
stack_top:

section .text.boot exec
global _start

_start: 
	cli
	
multiboot_entry:
%ifdef GRUB
	lgdt [_gdtr]
	jmp 0x08:pstart
	
pstart:
	mov cx, 0x10 

	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
%endif ; GRUB

	; When you assign the stack segment, you should consider the interrupt. So, after set the ss, you just immediately have to assign the esp and ebp.
	mov ss, cx
	mov ebp, stack_top	
	mov esp, stack_top
	
	mov ecx, cr4
	or ecx, 1<<4
	mov cr4, ecx

	mov [pde], dword 0x00000083			; Identify Mapped [0x100000:0x400000]
	;mov [pde+768*4], dword 0x00000083	; Identify Mapped [0xC0000000:0xC0400000]
	mov ecx, pde
	mov cr3, ecx	

	mov ecx, cr0
	or ecx, 1<<31
	mov cr0, ecx

	jmp higher_half_start

section .data.boot
align 4096
global pde
pde:
	dd 0x00000083
	times 767 dd 0x00000000
%assign i 0
%rep 256
	dd 0x00000083 + i
%assign i i+0x400000
%endrep

%ifdef GRUB
align 8
_gdt_start:
_gdt_tbl:
; NULL Segment
_gdt_null_desp:
    dw 0x0000
    dw 0x0000
    dw 0x0000
    dw 0x0000

; 32 Code Segment
_gdt32_code_desp:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

; 32 Data Segment
_gdt32_data_desp:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

; 64 Code Segment
_gdt64_code_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10011010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 1010 
    db 0b10101111   ; G = 1, D/B = 0, L = 1, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0
    
; 64 Data Segment
_gdt64_data_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b10010010   ; P = 1, DPL[6:5] = 0, S = 1, TYPE[3:0] = 0010 
    db 0b11001111   ; G = 1, D/B = 1, L = 0, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0
_gdt_end:

_gdtr:
    dw _gdt_end - _gdt_start - 1 
    dd _gdt_tbl

GDT32_CODE equ _gdt32_code_desp - _gdt_tbl
GDT32_DATA equ _gdt32_data_desp - _gdt_tbl
GDT64_CODE equ _gdt64_code_desp - _gdt_tbl
GDT64_DATA equ _gdt64_data_desp - _gdt_tbl

%endif ; GRUB
%endif
