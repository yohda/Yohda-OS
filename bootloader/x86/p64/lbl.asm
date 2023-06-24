%ifndef _LBL_NASM_
%define _LBL_NASM_

bits 32

PAGE_IA32_EFER_ADDR equ 0xC0000080

; data section
section .data.page64 write
align 4096
page_pml4:
	times 1024 dd 0

align 4096
page_low_pdtp:
	dd 0x00000003
	dd 0x00000000
	times 1022 dd 0

page_low_pd:
	dd 0x00000083
	dd 0x00000000
	times 1022 dd 0

;;;;; high page table
;align 4096
;page_high_pdtp:
;	dd 0x00000083
;	dd 0x00000000
;	times 1022 dd 0 	
		
align 4096
page_pml5:

section .text.mode64 exec nowrite
global _start_64:

_start_64:
	; Disable Paging
	mov eax, cr0
	and eax, ~(1<<31)
	mov cr0, eax	

	; Enable PAE
	mov eax, cr4
	or eax, 1<<5	; CR4[5] = PAE
	mov cr4, eax

	; Load Paging Table
	mov dword [page_pml4], page_low_pdtp + 0x00000003
	mov dword [page_low_pdtp], page_low_pd + 0x00000003
	mov dword [page_low_pd], 0x00000083
	mov eax, page_pml4
	mov cr3, eax

	;mov dword [0x106000], 0x107000 + 0x00000003 ; PML4 = 0x106000
	;mov dword [0x107000], 0x108000 + 0x00000003 ; PDPTE = 0x107000
	;mov dword [0x108000], 0x00000083 			; PDE = 0x108000 ; identify mapped the first 2MB page
	;mov dword [0x108000 + 8], 0x00000083		; identify mapped the second 2MB page
	;mov eax, 0x106000
	;mov cr3, eax

	; Enable Long mode
	mov ecx, PAGE_IA32_EFER_ADDR
	rdmsr
	or eax, 1<<8	; EFER[8] = LME
	wrmsr
	
	; Re-enable Long mode
	mov eax, cr0
	or eax, 1<<31
	mov cr0, eax

	mov eax, 2
	mov ebx, 3
	mov edx, 4
	;jmp _lmode
	jmp 0xFFFF800000100000
	;jmp _lmode

section .text
_lmode:
	jmp $
	
 
%endif 
