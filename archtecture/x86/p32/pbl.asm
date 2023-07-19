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
global higher_half_start, _tss

higher_half_start:
	mov dword [pdes+0], 0
	invlpg [pdes+0] 	
	
	mov ecx, _tss
	mov word [_tss_desc+2], cx

	shr ecx, 16
	mov byte [_tss_desc+4], cl
	
	shr ecx, 8
	mov byte [_tss_desc+7], cl

	lgdt [_gdtr]
	jmp KER32_CODE:gdt_load

gdt_load:
	mov cx, KER32_DATA 

	mov ds, cx
	mov es, cx
	mov fs, cx
	mov gs, cx
	
	mov ss, cx
	mov ebp, stack_top
	mov esp, stack_top

	; Load TSS	
	xor cx, cx
	mov cx, _tss_desc - _gdt_tbl
	ltr cx

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
_ker32_code_desp:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10011010b
    db 11001111b
    db 0x00

; 32 Data Segment
_ker32_data_desp:
    dw 0xFFFF
    dw 0x0000
    db 0x00
    db 10010010b
    db 11001111b
    db 0x00

; 64 Code Segment
_usr32_code_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b11111010   ; P = 1, DPL[6:5] = 3, S = 1, TYPE[3:0] = 1010 
    db 0b10101111   ; G = 1, D/B = 0, L = 1, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0
    
; 64 Data Segment
_usr32_data_desp:
    dw 0xFFFF       ; limit = 0
    dw 0x0000       ; base1[16] = 0
    db 0x00         ; base2[8] = 0
    db 0b11110010   ; P = 1, DPL[6:5] = 3, S = 1, TYPE[3:0] = 0010 
    db 0b11001111   ; G = 1, D/B = 1, L = 0, AVL = 0, LIMIT[3:0] = 0 
    db 0x00         ; base3[8] = 0

_tss_desc:
	dw (_tss_end - _tss) - 1
	dw 0x0000 ; base0
	db 0x00 ; base1
	db 0b10001001 ; P=1, DPL=00, 0, TYPE=1001
	db 0b00000000 ;
	db 0x00 ; base2
_tss_desc_end:
_gdt_end:

_gdtr:
    dw _gdt_end - _gdt_start - 1 
    dd _gdt_tbl

KER32_CODE equ _ker32_code_desp - _gdt_tbl
KER32_DATA equ _ker32_data_desp - _gdt_tbl
USR32_CODE equ _usr32_code_desp - _gdt_tbl
USR32_DATA equ _usr32_data_desp - _gdt_tbl

; for system call
; We have a plan to use the kernel stack and set esp0 and ss0 to `0x10`
_tss: 
	dw 0x0000 ; ptl 2
	dw 0x0000 ; rsvd 4
	dd stack_top ; esp0 8
	dw 0x10 ; ss0 10
	dw 0x0000 ; rsvd 12
	dd 0x00000000 ; esp1 16
	dw 0x0000 ; ss1 18
	dw 0x0000 ; rsvd 20
	dd 0x00000000 ; esp2 24
	dw 0x0000 ; ss2 26
	dw 0x0000 ; rsvd 28
	dd 0x00000000 ; cr3 32
	dd 0x00000000 ; eip 36
	dd 0x00000000 ; eflags 40
	dd 0x00000000 ; eax 44
	dd 0x00000000 ; ecx 48
	dd 0x00000000 ; edx 52
	dd 0x00000000 ; ebx 56
	dd 0x00000000 ; esp 60
	dd 0x00000000 ; ebp 64
	dd 0x00000000 ; esi 68
	dd 0x00000000 ; edi 72
	dw 0x0010 ; es 74
	dw 0x0000 ; rsvd 76
	dw 0x0008 ; cs 78
	dw 0x0000 ; rsvd 80
	dw 0x0010 ; ss 82
	dw 0x0000 ; rsvd 84
	dw 0x0010 ; ds 86
	dw 0x0000 ; rsvd 88
	dw 0x0010 ; fs 90
	dw 0x0000 ; rsvd 92
	dw 0x0010 ; gs 94
	dw 0x0000 ; rsvd 96
	dw 0x0000 ; ldt 98
	dw 0x0000 ; rsvd 100
	dw 0x0000 ; t, rsvd 102
	dw 0xFFFF ; i/o map base 104
_tss_end:

%endif 
