%ifndef _PAGING_NASM_
%define _PAGING_NASM_

bits 32

PAGE_KERN_VIRT_ADDR equ 0xC0000000
PAGE_DIR_KERN_ENTRY equ 768
PAGE_DIR_ENTRYS equ 1024
PAGE_IA32_EFER_ADDR equ 0xC0000080
;PAGE_PHY_LONG_ENTRY_POINT equ 0x100000

; data section
section .data
align 4096
page_pml3:
	; To-do

align 4096
page_pml2:
	; To-do

align 4096
page_pml4:
	times 1024 dd 0
	
align 4096
page_pdtp:
	dd 0x00000083
	dd 0x00000000
	times 1022 dd 0 	
		
align 4096
page_pml5:

; text section
section .text
global page_lmode_enable

align 4
page_lmode_enable:
	pushad
	
	; Disable Paging
	mov eax, cr0
	and eax, ~(1<<31)
	mov cr0, eax	

	; Enable PAE
	mov eax, cr4
	or eax, 1<<5	; CR4[5] = PAE
	mov cr4, eax

	; Load Paging Table
	;mov dword [page_pml4+512*4], page_pdtp
	;or dword [page_pml4+512*4], 0x00000003
	;mov eax, page_pml4
	;mov cr3, eax

	; Enable Long mode
	mov ecx, PAGE_IA32_EFER_ADDR
	rdmsr
	or eax, 1<<8	; EFER[8] = LME
	wrmsr

	; Re-enable Long mode
	;mov eax, cr0
	;or eax, 1<<31
	;mov cr0, eax
	
	;jmp $
	popad

	jmp _lmode
	;jmp 0xFFFF800000100000
	;jmp _lmode

[bits 64]
_lmode:
	;jmp 0xFFFF800000100000
	jmp 0x100000	

%endif 
