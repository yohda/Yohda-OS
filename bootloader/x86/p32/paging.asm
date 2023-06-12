%ifndef _PAGING_NASM_
%define _PAGING_NASM_

[BITS 32]

PAGE_KERN_VIRT_ADDR equ 0xC0000000
PAGE_DIR_KERN_ENTRY equ 768
PAGE_DIR_ENTRYS equ 1024

global load_higher_half

SECTION .data

align 4096
iden_page_dir:
	times PAGE_DIR_ENTRYS dd 0x00000000

SECTION .text
align 4

load_higher_half:
	mov eax, iden_page_dir - PAGE_KERN_VIRT_ADDR
	mov dword [eax + (PAGE_DIR_KERN_ENTRY * 4)], 0x00000083

	; load page directory locaton to cr3
	mov cr3, eax

	; set 31-bit of cr0 to enable paging
	mov eax, cr0
	or eax, 0x80000000	
	mov cr0, eax
		
%endif 
