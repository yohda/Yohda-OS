%ifndef _PAGING_NASM_
%define _PAGING_NASM_

bits 32

extern main
extern pde

section .text
global higher_half_start

higher_half_start:
	;mov [pde], dword 0 
	;invlpg [0] 	

	push ebx ; push the pointer to multiboot information structure
	push eax ; push the magic value
	
	call main
	
	jmp $				; Nevere come here
	
%endif 
