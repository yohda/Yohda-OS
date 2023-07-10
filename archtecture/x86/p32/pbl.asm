%ifndef _PBL_NASM_
%define _PBL_NASM_

[bits 32]

extern main
extern pdes

VIRT_BASE equ 0xC0000000
PHY_PDE_ADDR equ pdes

extern CPUID_PAGE_4MB

section .bss
align 16
stack_bottom:
resb 1024*32 ; 32KB
stack_top:

section .text
global higher_half_start

higher_half_start:
	mov dword [pdes+0], 0
	invlpg [pdes+0] 	

	lgdt [_gdtr]
	jmp GDT32_CODE:gdt_load

gdt_load:
	mov cx, GDT32_DATA 

	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	
	mov ss, cx
	mov ebp, stack_top
	mov esp, stack_top
	
	push ebx ; push the pointer to multiboot information structure
	push eax ; push the magic value
	
	call main
	
	jmp $				; Nevere come here
	
section .data
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

%endif 
