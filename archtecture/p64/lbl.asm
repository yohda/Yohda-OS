%ifndef _LBL_ASM_
%define _LBL_ASM_

[bits 32]

PAGE_IA32_EFER_ADDR equ 0xC0000080

; data section
section .data.page64 write
align 4096
page_pml4:
	times 1024 dd 0

; Load Page Table
align 4096
page_temp_pdtp:
	times 1024 dd 0

page_temp_pd:
	times 1024 dd 0

; High Page Table
align 4096
page_high_pdtp:
	times 1024 dd 0

;align 4096
;page_high_pd:
;	times 1024 dd 0 	

align 4096
page_fhigh_pd: ; 1GB
	times 1024 dd 0 

;align 4096
;page_high_pt1:
;%rep 512
;%assign i 0
;%rep 512
;	dd 0x00000003+i
;	dd 0x00000000
;%assign i i+0x1000
;%endrep
;%endrep

align 4096
page_high_pt2:
%assign i 0
%rep 512
	dd 0x00000003+i
	dd 0x00000000
%assign i i+0x1000
%endrep

align 4096
page_high_pt3:
%assign i 0
%rep 512
	dd 0x00000003+i
	dd 0x00000000
%assign i i+0x1000
%endrep

align 4096
page_high_pt4:
%assign i 0
%rep 512
	dd 0x00000003+i
	dd 0x00000000
%assign i i+0x1000
%endrep

align 4096
page_high_pt5:
%assign i 0
%rep 512
	dd 0x00000003+i
	dd 0x00000000
%assign i i+0x1000
%endrep

align 4096
page_high_pt6:
%assign i 0
%rep 512
	dd 0x00000003+i
	dd 0x00000000
%assign i i+0x1000
%endrep

align 4096
page_shigh_pd: ; 1GB
%assign i 0
%rep 512
	dd 0x00000083+i
	dd 0x00000000
%assign i i+0x200000
%endrep

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

	; Load 4MB Low Temporary Identify Mapped Paging Table
	mov dword [page_pml4], page_temp_pdtp + 0x00000003
	mov dword [page_temp_pdtp], page_temp_pd + 0x00000003
	mov dword [page_temp_pd], 0x00000083
	mov dword [page_temp_pd+8], 0x00200083

	; Load 2GB Higher-Half Identify Mapped Paging Table
	
;%assign i 0
;%assign j 0
;%rep 512
;	mov eax, page_high_pt1
;	add eax, j + 0x00000003
;	mov dword [page_fhigh_pd + i], eax
;%assign i i+8
;%assign j j+4096 
;%endrep

	mov dword [page_fhigh_pd], page_high_pt2 + 0x00000003
	mov dword [page_fhigh_pd+8], page_high_pt3 + 0x00000003
	mov dword [page_fhigh_pd+16], page_high_pt4 + 0x00000003
	mov dword [page_fhigh_pd+24], page_high_pt5 + 0x00000003
	mov dword [page_fhigh_pd+32], page_high_pt6 + 0x00000003

	; 510 & 511 PDTPE`s setting.	
	mov dword [page_high_pdtp+510*2*4], page_fhigh_pd + 0x00000003
	mov dword [page_high_pdtp+511*2*4], page_shigh_pd + 0x00000003
	
	; 511 PML4E`s setting. 
	mov dword [page_pml4+511*2*4], page_high_pdtp + 0x00000003

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

	jmp 0x18:_trampoline64

[bits 64]
extern _lenter
_trampoline64:
	jmp _lenter

%endif 
