%ifndef _PBL_NASM_
%define _PBL_NASM_

[BITS 32]

extern main

SECTION .text
global _pbl_start

_pbl_start: 
	cli

	; After entering into protected mode, you just should load data segment into each data segments.
	mov ax, 0x10 

	mov ds, ax
	mov es, ax
	mov fs, ax
	mov gs, ax

	; When you assign the stack segment, you should consider the interrupt. So, after set the ss, you just immediately have to assign the esp and ebp.
	mov ss, ax
	mov ebp, 0x80000	
	mov esp, 0x80000

	call main
	jmp $				; Nevere come here

%endif
