%ifndef _LBL_ASM_
%define _LBL_ASM_

bits 64

section .text
global load_higher_half

load_higher_half:
	;mov eax, iden_page_dir - PAGE_KERN_VIRT_ADDR
	;mov dword [eax + (PAGE_DIR_KERN_ENTRY * 4)], 0x00000083

	; load page directory locaton to cr3
	;mov cr3, eax

	; Re-enable Paging
	;mov eax, cr0
	;or eax, (1<<31)|(1<<0)
	
	;mov cr0, eax

		
%endif
