%ifndef _MODE64_ASM_
%define _MODE64_ASM_

[bits 64]
extern main

global _lenter

section .text
_lenter:
	;mov dword [page_low_pd], 0
	;mov dword [page_low_pd+8], 0 
	;mov dword [page_low_pdtp], 0
	;mov dword [page_pml4], 0
	
	;invlpg [0]
	
	mov rax, 0x20

	mov ds, rax
	mov es, rax
	mov fs, rax
	mov gs, rax
	
	mov ss, rax
	mov rbp, 0x200000 ; 32bit end point
	mov rsp, 0x200000 ; 32bit end point

	call main

%endif 
